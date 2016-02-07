# Mini interactive cube

The initial cube was made using led panels on each sides, but it lacks one important piece, input, and only having orientation/acceleration is not optimal, this cube addresses that, and is slightly different, it has sixteen buttons on each of the six sides and they are all usable through OSC.

![Enclosure](doc/enclosure.png)



## OSC Messages

The device sends and receives OSC messages, these are the different ones.


### Key press

`/cube1/btn btn down`

Namespace: `/cube1/btn`

Arguments:
  `btn` int32 - Which button
  `down` int32 - 1 = Button was pressed, 0 = Button was released


### Gyro data

`/cube1/rot rx ry rz`

Namespace: `/cube1/rot`

Arguments:
  `rx` int32 - Relative rotation around X.
  `ry` int32 - Relative rotation around Y.
  `rz` int32 - Relative rotation around Z.


### Accelerometer data

`/cube1/acc ax ay az`

Namespace: `/cube1/acc`

Arguments:
  `ax` int32 - Relative acceleration X.
  `ay` int32 - Relative acceleration Y.
  `az` int32 - Relative acceleration Z.



### Update entire display

`/cube1/leds n0 n1 n2 n3 n4 n5 n6 n7 n8 n9 n10 n11`

Namespace: `/cube1/leds`

Arguments:
  `n0..n11` int32 - Led pixels, 12 8bit values - Two values for each side, 16 bits, 16 leds.


### Update one side

`/cube1/leds side n0 n1`

Namespace: `/cube1/leds`

Arguments:
  `side` int32 - Which side to update.
  `n0`, `n1` int32 - Two 8bit values, 16 bits, 16 leds.


### Update one pixel(Sent to

`/cube1/leds led on`

Namespace: `/cube1/leds`

Arguments:
  `n0..n11` int32 - Led pixels, two values for each side, 16 bits, 16 leds.









