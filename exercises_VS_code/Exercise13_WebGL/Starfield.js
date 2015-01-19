"use strict";

//**************************
//* Starfield class
//**************************
function Starfield(type, pos, radius, velocity, rotVelocity)
{
    //create model matrix, scale and translate a unit quad to fit the entire screen (canvas)
    var model = mult(translate([canvas.width / 2, canvas.height/2, 0]), scale(canvas.width, canvas.height, 0));

    //create modelview matrix
    this.ModelView = mult(view, model);
}

//draws a screen size quad using the starfield shader
Starfield.prototype.Draw = function(elapsed)
{
    //use starfield shader program
    gl.useProgram(starfieldShader);

    //set uniforms (projection, modelview, time, minimum canvas resolution)
    gl.uniformMatrix4fv(gl.getUniformLocation(starfieldShader, "Projection"), false, flatten(projection));
    gl.uniformMatrix4fv(gl.getUniformLocation(starfieldShader, "ModelView"), false, flatten(this.ModelView));
    gl.uniform1fv(gl.getUniformLocation(starfieldShader, "Time"), [frame]);
    gl.uniform1fv(gl.getUniformLocation(starfieldShader, "MinResDim"), [Math.min(canvas.width, canvas.height)]);

    //draw
    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
};