# вычисление первой производной в точке (u, v)
r_u = [ d(x, u), d(y, u), d(z, u) ]
r_v = [ d(x, v), d(y, v), d(z, v) ]

# вычисление второй производной в точке (u, v)
r_uu = [ d(d(x, u), u), d(d(y, u), u), d(d(z, u), u) ]
r_uv = [ d(d(x, u), v), d(d(y, u), v), d(d(z, u), v) ]
r_vv = [ d(d(x, v), v), d(d(y, v), v), d(d(z, v), v) ]

# нахождение нормали
n = cross(r_u, r_v)

# длина нормали
length = abs(n)

# нормализация
n = [ n[1] / length, n[2] / length, n[3] / length ]

# вычисление первой квадратичной формы
g_11 = simplify(r_u[1] * r_u[1] + r_u[2] * r_u[2] + r_u[3] * r_u[3])
g_12 = simplify(r_u[1] * r_v[1] + r_u[2] * r_v[2] + r_u[3] * r_v[3])
g_22 = simplify(r_v[1] * r_v[1] + r_v[2] * r_v[2] + r_v[3] * r_v[3])

# вычисление второй квадратичной формы
q_11 = simplify(r_uu[1] * n[1] + r_uu[2] * n[2] + r_uu[3] * n[3])
q_12 = simplify(r_uv[1] * n[1] + r_uv[2] * n[2] + r_uv[3] * n[3])
q_22 = simplify(r_vv[1] * n[1] + r_vv[2] * n[2] + r_vv[3] * n[3])

# вычисление гауссовой кривизны
f = simplify((q_11 * q_22 - q_12 * q_12) / (g_11 * g_22 - g_12 * g_12))