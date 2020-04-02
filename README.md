# 3d-pinball

Physics Pinball

## JSON Format

### gameObjects.json

```json
{
    ...
    "Physics":
    {
        "shape": "sphere" | "box" | "cylinder" | "mesh",
        // if not specified, a rigid body will not be created
        "mass": 10.0, // optional, default to 0 (static)
        // for sphere and cylinder
        "radius": 3.0,
        // for box 
        "height": 10.0, // full height (Y axis)
        "width": 8.0, // full width (X axis)
        // for box and cylinder (z oriented cylinder)
        "depth": 2.0, // full depth (Z axis)
        // the mesh shape will use the mesh data (maybe)
        // The rigid body starting position and rotation
        // will be the same as the game object
    },
    ...
    // if colliding with this object gives the player points
    "givesPoints": true
}
```

## TODO

- Spring Launcher (Maybe scale one axis independent)
- Sphere 6 DoF
