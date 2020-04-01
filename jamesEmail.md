I think the 3D pinball would be a great idea.

Your proposal mentions Capsules, AABB, and OBB.  I will want more than that.

AABB and OBB shouldn't be used as in-game shapes. Both should be btBoxShape.

In your requirements list for shapes, I will like to see btSphereShape, btBoxShape, btCylinderShape, and btConvexHullShape.  

I expect the configuration will change, but to explain how each shape should be used I'm looking at the second image in your proposal, and I am seeing these shapes:

    The pinball: btSphereShape
    The overall boundaries: btBoxShape (x5) (1 floor, 4 walls)
    At the top center: btBoxShape (x7)
    Below the boxes: btCylinderShape (x3)
    The bumpers:  btBoxShape (x2) or btConvexHullShape (x2)
    The upper light blue boundaries: btConvexHullShape (x2)
    The lower light blue boundaries: btBoxShape(x2) *These are the triangle shapes, but setting two fixed boxes at a 45degree angle will accomplish the same thing more efficiently... keep in mind the graphics can still be triangles :) 
    The gray angle pieces left and right: btBoxShape (x4)
    The red triangle pieces left and right: btConvexHullShape (x2)
    (Unseen) The launcher: btCylinderShape

In your requirements list for constraints, I will like to see btHingeConstraint, btGeneric6dofConstraint, and btSliderConstraint.

    For your bumpers: btHingeConstraint (x2) with appropriate max/min angles
    For your pinball: btGeneric6dofConstraint to lock it in vertically on the board
    For your launcher (seen on the right side of the "Space Cadet" image): btSliderConstraint

For implementation, I am going to make these implementation suggestions that should make things much easier:

    Consider the board to be flat in the x-z dimensions, and position everything accordingly.  Set the gravity of the world to on an angle so the pinball will always accelerate towards the bottom ( something like <0,-9,2> )  This means you don't have to worry about rotations about the y axis, and when you decide to move elements around on your board you don't have to keep recalculating y-values (I'll discuss this on Tuesday in case that doesn't make much sense right now lol pictures really do help)

In addition I will want to see these requirements:

    Point system implementation:  collision with various elements are observed and points are accumulated (eg. 5 points for those squares, 10 for the circles, 1 for the outside boundaries above the bumpers, etc..).  There is feedback so the player knows points are accumulating (trigger sounds, render point text on screen).
    Basic game cycle implementation: The player is allowed 3 lives. The player starts with the pinball in the launcher position.  Collision detection past the bumpers triggers a life lost and resetting the player to the launcher position.  When the 3 lives are lost, the game is over, feedback is given to the player, and another game may begin.
    Collision filtering is implemented to optimize the physics within bullet (See Section 5 of Bullet's Manual, or look up "collision filtering")  The pinball may collide with everything, and nothing else collides!

Your Next Steps:

    Let me know what you think about all of that.  Does it sound challenging, but doable? Does it sound crazy?
    Update your proposal. Add a section "Requirements" that lists all the requirements and marks you want for each one.
    Send me the updated proposal.
