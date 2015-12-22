Ray tracer
==========

A ray tracing program with diffuse and specular reflection.
Uses triangles shperes and point lights.
Images are output in PNM binary format (P6).

Useage
------

`make && ./ray` or `make run`.
The file is printed to `stdout` by default.
`make disp` attempts to use the tool `pnmtopng` from Netpbm to convert the image to a png, then display it with the image veiwer `eog`.
