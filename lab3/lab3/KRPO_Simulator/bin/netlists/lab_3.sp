
v1 1 0 pulse 0 5 10n 5n 5n 50n 100n
r1 1 2 5
d1 2 0
c1 2 3 8p
r2 3 0 5K
.tran 1n 100n
*.plot v(1) v(2) v(3) i(d1:1)
.end