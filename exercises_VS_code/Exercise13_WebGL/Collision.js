"use strict";

//Checks if an object is out of bounds
function OutOfBounds(entity)
{
    if (entity.Pos[0] + entity.Radius*4 < 0 || entity.Pos[0] - entity.Radius*4 > canvas.width ||
        entity.Pos[1] + entity.Radius*4 < 0 || entity.Pos[1] - entity.Radius*4 > canvas.height)
        return true;

    return false;
}

//Applies elastic collision response to A and B
//Requires A and B to have Velocity and Radius properties
function ElasticCollisionResponse(A, B)
{
    //velocity reponse based on simple elastic collision (http://en.wikipedia.org/wiki/Elastic_collision)

    //the new velocities of A and B are
    //   vA = (u1 * (m1-m2) + 2*m2*u2) / (m1+m2)
    //   vB = (u2 * (m2-m1) + 2*m1*u1) / (m1+m2)
    //where u is velocity and m is mass
    //we use the raidus of A and B as thier mass



    //compute the new velocities
    var velAx = (A.Velocity[0] * (A.Radius - B.Radius) + 2*B.Radius * B.Velocity[0])/(A.Radius + B.Radius);
    var velAy = (A.Velocity[1] * (A.Radius - B.Radius) + 2 * B.Radius * B.Velocity[1]) / (A.Radius + B.Radius);
    var velBx = (B.Velocity[0] * (B.Radius - A.Radius) + 2 * A.Radius * A.Velocity[0]) / (A.Radius + B.Radius);
    var velBy = (B.Velocity[1] * (B.Radius - A.Radius) + 2 * A.Radius * A.Velocity[1]) / (A.Radius + B.Radius);


    //apply collision reponse, that is we move out of the collision along the new trajectory
    A.Pos[0] += velAx;
    A.Pos[1] += velAy;
    B.Pos[0] += velBx;
    B.Pos[1] += velBy;

    //update A and B with the new velocity
    A.Velocity = [velAx, velAy, 0];
    B.Velocity = [velBx, velBy, 0];
}

//Circle-Circle collision detection
//Requires objects A and B to have Pos and Radius properties
function AsteroidCollision(A, B)
{
    var dist = length(subtract(A.Pos, B.Pos));

    return dist < A.Radius * 0.88 + B.Radius * 0.88;
}

//Check ast for collision vs all other asteroids starting from index startIdx
//Returns the index of the collided asteroid or -1 if no collision
function CheckAsteroidCollisionsSingleVsAll(ast, startIdx)
{
    for (var j = startIdx; j < Asteroids.length; j++)
    {
        if (AsteroidCollision(ast, Asteroids[j]))
        {
            //collision with asteroid j
            return j;
        }
    }

    //no collision
    return -1;
}

//Handles asteroid vs asteroid collisions
function HandleAsteroidAsteroidCollisions()
{
    for (var i = 0; i < Asteroids.length; i++)
    {
        //check the i'th asteroid for collision with all other asteroids with higher index
        var j = CheckAsteroidCollisionsSingleVsAll(Asteroids[i], i + 1);

        //if collision found
        if (j > -1)
        {
            //apply collision response
            ElasticCollisionResponse(Asteroids[i], Asteroids[j]);
        }
    }
}

//Handles shot vs asteroid collisions
function HandleShotAsteroidCollisions()
{
    for (var i = 0; i < Shots.length; i++)
    {
        //check if the i'th shot collides with any asteroid
        var j = CheckAsteroidCollisionsSingleVsAll(Shots[i], 0);

        //if collision
        if (j > -1)
        {
            //mark shot i and asteroid j for deletion
            Shots[i].MarkedForDeletion = true;
            Asteroids[j].MarkedForDeletion = true;

            //spawn explosion
            ParticleSystem.SpawnExplosion(Asteroids[j].Pos, (Asteroids[j].Radius * 2) / (AsteroidMaxRadius));

            //increase score
            GameState.Score += 10;

            //update gui
            updateGUI()
        }
    }
}

//Check player for collision vs all asteroids
function HandlePlayerAsteroidCollisions()
{
    //check if player collides with any asteroid
    var j = CheckAsteroidCollisionsSingleVsAll(Player, 0);

    //if collision
    if (j > -1)
    {
        //mark asteroid j for deletion
        Asteroids[j].MarkedForDeletion = true;

        //spawn exposion at asteroid
        ParticleSystem.SpawnExplosion(Asteroids[j].Pos, (Asteroids[j].Radius * 2) / (AsteroidMaxRadius));

        //decrease player life
        GameState.PlayerHP -= 1;

        //if player is dead
        if (GameState.PlayerHP <= 0)
        {
            //spawn explosion at player
            ParticleSystem.SpawnExplosion(Player.Pos, 3);

            //stop game loop
            GameState.IsRunning = false;
        }

        //update gui
        updateGUI();
    }
}

