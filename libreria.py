import os
import sys
import ctypes
#from cassandra.cluster import Cluster

sys.path.append('/home/frb2/outs')
# Antes de cargar nuestra libreria cargar cassandra
ctypes.cdll.LoadLibrary("outs/libcassandra.so.2")
libreria = ctypes.CDLL("outs/t_l.so")

# Variables de secuencias, kafka cargara estas variables
s1 = 0
r1 = 1
s2 = 1
r2 = 1

# Preparacion variables
libreria.residue_pair_extended_list.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int]
# Recepcion score
score = libreria.residue_pair_extended_list(s1, r1, s2, r2)
