Viewed Source.bb:171-970

Line 571 and lines immediately following it are responsible for creating and configuring the entity's dot on the minimap (radar).

Specifically, line 571 **instantiates** the dot by copying a template:
```bb
571:    p\dot = CopyEntity(HUD\dot , HUD\model)
```

The actual **color, size, and appearance** are assigned in the block of code directly below it (lines 574–578):

*   **Transparency (Alpha):** `EntityAlpha p\dot,(AI<>17)` (Line 576) sets the visibility. It looks like AI type 17 might be hidden or treated differently.
*   **Color:** `EntityColor p\dot,F\r,F\g,F\b` (Line 577) assigns the RGB color defined in the `FlyingObject` configuration for that entity type.
*   **Size (Scale):** `ScaleEntity p\dot,F\radius*HGW,F\radius*HGW,HGW` (Line 578) scales the dot based on the entity's `radius`.

In summary, line 571 creates the object, but the visual properties you asked about are defined in the 2-7 lines that follow it.