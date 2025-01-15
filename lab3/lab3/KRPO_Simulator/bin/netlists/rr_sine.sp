
*          v0 va freq  td  df phase
v1 1 0 sin 0  5  0.1g  10n 0  0
r1 1 2 1
r2 2 0 1
.tran 0.1n 100n
*.plot v(1) v(2)
.end