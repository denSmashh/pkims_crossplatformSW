
v1 1 0 pulse 0 5 10n 5n 5n 50n 100n
r1 1 2 1k
c1 2 0 1p
.tran 1n 100n
*.plot v(1) v(2)
.end