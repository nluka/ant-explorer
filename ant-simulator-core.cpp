#include <sstream>
#include <algorithm>
#include "ant-simulator-core.hpp"

using asc::Simulation, asc::Rule, asc::StepResult;

char const *asc::step_result_to_string(StepResult const res) {
  switch (res) {
    case StepResult::NIL:                return "nil";
    case StepResult::SUCCESS:            return "success";
    case StepResult::FAILED_AT_BOUNDARY: return "hit boundary";
    default: throw "bad StepResult";
  }
}
char const *asc::ant_orient_to_string(int_fast8_t const orientation) {
  switch (orientation) {
    case AO_NORTH: return "N";
    case AO_EAST:  return "E";
    case AO_SOUTH: return "S";
    case AO_WEST:  return "W";
    default: throw "bad orientation";
  }
}
char const *asc::turn_dir_to_string(int_fast8_t turnDir) {
  switch (turnDir) {
    case TD_LEFT:  return "L";
    case TD_NONE:  return "N";
    case TD_RIGHT: return "R";
    default: throw "bad turn direction";
  }
}

Rule::Rule()
: m_isDefined{false},
  m_replacementShade{},
  m_turnDirection{}
{}

Rule::Rule(
  uint8_t const replacementShade,
  int_fast8_t const turnDirection
) :
  m_isDefined{true},
  m_replacementShade{replacementShade},
  m_turnDirection{turnDirection}
{}

bool Simulation::is_coord_in_grid_dimension(
  int const coord,
  uint_fast16_t const gridDimension
) {
  return coord >= 0 && coord < static_cast<int>(gridDimension);
}

Simulation::Simulation()
: m_name{},
  m_gridWidth{0},
  m_gridHeight{0},
  m_grid{nullptr},
  m_antCol{0},
  m_antRow{0},
  m_antOrientation{},
  m_rules{},
  m_nextSingularSnapshotIdx{-1}
{}

Simulation::Simulation(
  std::string const &name,
  uint_fast64_t iterationsCompleted,
  uint_fast16_t const gridWidth,
  uint_fast16_t const gridHeight,
  uint8_t const gridInitialShade,
  uint_fast16_t const antStartingCol,
  uint_fast16_t const antStartingRow,
  int_fast8_t const antOrientation,
  std::array<Rule, 256> const &rules,
  std::vector<uint_fast64_t> &&singularSnapshots,
  std::vector<uint_fast64_t> &&periodicSnapshots
)
: m_name{name},
  m_iterationsCompleted{iterationsCompleted},
  m_gridWidth{gridWidth},
  m_gridHeight{gridHeight},
  m_grid{nullptr},
  m_antCol{antStartingCol},
  m_antRow{antStartingRow},
  m_antOrientation{antOrientation},
  m_rules{rules},
  m_singularSnapshots{singularSnapshots},
  m_periodicSnapshots{periodicSnapshots},
  m_nextSingularSnapshotIdx{-1}
{
  size_t const cellCount =
    static_cast<size_t>(gridWidth) * static_cast<size_t>(gridHeight);

  // TODO: maybe handle std::bad_alloc?
  m_grid = new uint8_t[cellCount];

  std::fill_n(m_grid, cellCount, gridInitialShade);
}

std::string const &Simulation::name() const {
  return m_name;
}
uint_fast64_t Simulation::iterations_completed() const {
  return m_iterationsCompleted;
}
StepResult Simulation::last_step_result() const {
  return m_mostRecentStepResult;
}
uint_fast16_t Simulation::grid_width() const {
  return m_gridWidth;
}
uint_fast16_t Simulation::grid_height() const {
  return m_gridHeight;
}
uint_fast16_t Simulation::ant_col() const {
  return m_antCol;
}
uint_fast16_t Simulation::ant_row() const {
  return m_antRow;
}
int_fast8_t Simulation::ant_orientation() const {
  return m_antOrientation;
}
uint8_t const *Simulation::grid() const {
  return m_grid;
}
bool Simulation::is_finished() const {
  return m_mostRecentStepResult > StepResult::SUCCESS;
}

void Simulation::step_foward() {
  size_t const currCellIdx = (m_antRow * m_gridWidth) + m_antCol;
  uint8_t const currCellShade = m_grid[currCellIdx];
  auto const &currCellRule = m_rules[currCellShade];

  // turn
  m_antOrientation = m_antOrientation + currCellRule.m_turnDirection;
  if (m_antOrientation == AO_OVERFLOW_COUNTER_CLOCKWISE) {
    m_antOrientation = AO_WEST;
  } else if (m_antOrientation == AO_OVERFLOW_CLOCKWISE) {
    m_antOrientation = AO_NORTH;
  }

  // update current cell shade
  m_grid[currCellIdx] = currCellRule.m_replacementShade;

  { // try to move to next cell
    int nextCol;
    if (m_antOrientation == AO_EAST) {
      nextCol = static_cast<int>(m_antCol) + 1;
    } else if (m_antOrientation == AO_WEST) {
      nextCol = static_cast<int>(m_antCol) - 1;
    } else {
      nextCol = static_cast<int>(m_antCol);
    }

    int nextRow;
    if (m_antOrientation == AO_NORTH) {
      nextRow = static_cast<int>(m_antRow) - 1;
    } else if (m_antOrientation == AO_SOUTH) {
      nextRow = static_cast<int>(m_antRow) + 1;
    } else {
      nextRow = static_cast<int>(m_antRow);
    }

    if (
      !is_coord_in_grid_dimension(nextCol, m_gridWidth) ||
      !is_coord_in_grid_dimension(nextRow, m_gridHeight)
    ) {
      m_mostRecentStepResult = StepResult::FAILED_AT_BOUNDARY;
    } else {
      m_antCol = static_cast<uint16_t>(nextCol);
      m_antRow = static_cast<uint16_t>(nextRow);
      m_mostRecentStepResult = StepResult::SUCCESS;
      ++m_iterationsCompleted;
    }
  }
}
