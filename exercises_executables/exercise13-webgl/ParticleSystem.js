"use strict";

//return random float between min and max
function RandomBetween(min, max)
{
    return min + Math.random() * (max - min);
}


//**************************
//* ParticleSystem class
//**************************
function ParticleSystem()
{
    //active particles
    this.Particles = [];

    //inactive (free) particles
    this.ParticlePool = [];
}

//init particle system creating a pool of inactive particles
ParticleSystem.prototype.Initialize = function(totalParticles)
{
    for (var i = 0; i < totalParticles; i++) {
        this.ParticlePool.push(new Particle());
    }

    //TODO: push 'totalParticles' new particles into the particle pool

};

//update all particles and move inactive ones back to pool
ParticleSystem.prototype.Update = function(elapsed)
{
    //TODO: update all active particles
    for (var i = 0; i < this.Particles.length; i++) {
        this.Particles[i].Update(elapsed);
    }

    //remove dead particles from Particles and put back into ParticlePool
    this.Particles = this.Particles.filter(function(part)
    {
        if (part.Active === false)
        {
            ParticleSystem.ParticlePool.push(part);
            return false;
        }
        else
        {
            return true;
        }
    });
};

//draw all active particles
ParticleSystem.prototype.Draw = function(elapsed)
{
    //TODO: enable additive blending
    gl.blendEquation(gl.FUNC_ADD);


    //TODO: draw all active particles
    for (var i = 0; i < this.Particles.length; i++) {
        this.Particles[i].Draw(elapsed);
    }
};

//spawn explosion, create a number of particles
ParticleSystem.prototype.SpawnExplosion = function (pos, scaleMod)
{
    for (var i = 0; i < 30; i++)
    {
        //grab a free particle from the pool
        var p = this.ParticlePool.pop();

        //if we got a free particle then
        if (p)
        {
            //initialize particle with random values
            var life = RandomBetween(100, 200);
            var theta = RandomBetween(0, 2 * Math.PI);
            var speed = RandomBetween(0.05, 0.08);
            var vel = [Math.cos(theta) * speed, Math.sin(theta) * speed, 0];
            var scale = RandomBetween(32, 64) * scaleMod;
            var rotSpeed = RandomBetween(0.08, 0.12);
            var acc = 0;
            var color = [1, 1, 1, 1];


            //reset the particle p with the initial values and activate it
            p.Reset(pos, scale, vel, [acc, acc, 0], 0, rotSpeed, life, color);
            p.Active = true;

            //add p to active particles list
            this.Particles.push(p);
        }
    }

};



//**************************
//* Particle class
//**************************
function Particle()
{
    this.Pos = [0, 0, 0];
    this.Scale = 1;
    this.Velocity = [0, 0, 0];
    this.Acceleration = [0, 0, 0];
    this.Rotation = 0;
    this.RotationSpeed = 0;
    this.LifeTime = 0;
    this.Color = [1, 1, 1, 1];
    this.TimeSinceBirth = 0; //age
    this.Active = false;
}

//sets all particle properties
Particle.prototype.Reset = function (pos, scale, velocity, acceleration, rotation, rotationSpeed, lifeTime, color)
{

    this.Pos[0] = pos[0] || 0;
    this.Pos[1] = pos[1] || 0;
    this.Pos[2] = pos[2] || 0;

    this.Scale = scale || 1;

    this.Velocity[0] = velocity[0] || 0;
    this.Velocity[1] = velocity[1] || 0;
    this.Velocity[2] = velocity[2] || 0;

    this.Acceleration[0] = acceleration[0] || 0;
    this.Acceleration[1] = acceleration[1] || 0;
    this.Acceleration[2] = acceleration[2] || 0;

    this.Rotation = rotation || 0;
    this.RotationSpeed = rotationSpeed || 0;
    this.LifeTime = lifeTime || 0;

    this.Color[0] = color[0] || 1;
    this.Color[1] = color[1] || 1;
    this.Color[2] = color[2] || 1;

    this.TimeSinceBirth = 0;
    this.Active = false;
};

//updates particle, dt is the time since last update
Particle.prototype.Update = function(dt)
{
    //update velocity, v += a*t
    this.Velocity[0] += this.Acceleration[0] * dt; 
    this.Velocity[1] += this.Acceleration[1] * dt;
    this.Velocity[2] += this.Acceleration[2] * dt;

    //update position, p += v*t
    this.Pos[0] += this.Velocity[0] * dt;
    this.Pos[1] += this.Velocity[1] * dt;
    this.Pos[2] += this.Velocity[2] * dt;

    //update rotation, r += rs*t
    this.Rotation += this.RotationSpeed * dt;

    //increase age (TimeSinceBirth)
    this.TimeSinceBirth++;

    //update color (fade to transparent over life time)
    
    this.Color[3] = 1- this.TimeSinceBirth/this.LifeTime;


    //if particle is too old, deactivate it
    if (this.TimeSinceBirth >= this.LifeTime)
        this.Active = false;
};

//draws particle sprite
Particle.prototype.Draw = function(elapsed)
{
    //create model matrix
    var model = mult(translate(this.Pos), mult(scale(this.Scale, this.Scale, this.Scale), rotate(this.Rotation, vec3(0, 0, 1))));

    //set uniforms
    gl.uniformMatrix4fv(gl.getUniformLocation(spriteShader, "ModelView"), false, flatten(mult(view, model)));
    gl.uniform4fv(gl.getUniformLocation(spriteShader, "OverlayColor"), this.Color);
    gl.uniform1fv(gl.getUniformLocation(spriteShader, "SpriteIndex"), [2]); //index 2 is the flame'ish sprite

    //draw
    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
};
