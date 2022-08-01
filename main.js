(function main() {
  const canvas = document.querySelector("canvas");
  const gl = canvas.getContext("webgl");

  if (gl === null) {
    throw new Error("WebGL not supported");
  }

  // triangle
  const vertices = new Float32Array([0, 1, 0, /**/ 1, -1, 0, /**/ -1, -1, 0]);

  const buffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

  const vertexShader = gl.createShader(gl.VERTEX_SHADER);
  gl.shaderSource(
    vertexShader,
    `
    attribute vec3 position;
    void main() {
      gl_Position = vec4(position, 1);
    }
    `
  );
  gl.compileShader(vertexShader);

  const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
  gl.shaderSource(
    fragmentShader,
    `
    void main() {
      gl_FragColor = vec4(1, 0, 0, 1);
    }
    `
  );
  gl.compileShader(fragmentShader);

  const program = gl.createProgram();
  gl.attachShader(program, vertexShader);
  gl.attachShader(program, fragmentShader);
  gl.linkProgram(program);

  const positionLoc = gl.getAttribLocation(program, "position");
  gl.enableVertexAttribArray(positionLoc);
  gl.vertexAttribPointer(positionLoc, 3, gl.FLOAT, false, 0, 0);

  gl.useProgram(program);
  gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 3);
})();
