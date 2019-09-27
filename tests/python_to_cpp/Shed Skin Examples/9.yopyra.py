#
### to change the scene/resolution, edit scene.txt
#
# GPL Notice:
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Name:        YOPyRa (Yeah!, One Python Raytracer)
# Copyright:   Carlos Gonzalez Morcillo 2004, 2005, 2006
# Email:       carlos@morcy.org - Carlos.Gonzalez@uclm.es
# http://www.boxel.info/morcy/static.php?page=yopyra

import math
import sys
from typing import Optional

MAX_DIST = 1999999999.0 # 9999999999
PI_SOBRE_180 = 0.017453292
PEQUENO = 0.000000001

class Vector:
    x : float
    y : float
    z : float

    def __init__(self, vx=0.0, vy=0.0, vz=0.0):
        (self.x, self.y, self.z) = (vx, vy, vz)
#     def set(self, vx, vy, vz):
#         self.x, self.y, self.z = vx, vy, vz

    def pEscalar(self, vv):
        return (self.x * vv.x + self.y * vv.y + self.z * vv.z)
    def pVectorial(self, vv):
        r = Vector()
        r.x = vv.y*self.z - vv.z*self.y
        r.y = vv.z*self.x - vv.x*self.z
        r.z = vv.x*self.y - vv.y*self.x
        return r
    def modulo(self):
        return math.sqrt(self.x*self.x + self.y*self.y + self.z*self.z)
    def normalizar(self):
        m = self.modulo()
        if m != 0.0:
            self.x /= m;  self.y /= m; self.z /= m
        return self

    def __add__(self, other):
        return Vector(self.x+other.x, self.y+other.y, self.z+other.z)
    def __sub__(self, other):
        return Vector(self.x-other.x, self.y-other.y, self.z-other.z)
    def __mul__(self, other):
        return Vector(self.x*other, self.y*other, self.z*other)
#     def __idiv__(self, other):
#         return Vector(self.x / float(other), self.y / float(other), self.z / float(other))
#     def __iadd__(self, other):
#         return Vector(self.x + other.x, self.y + other.y, self.z + other.z)
#     def __repr__(self):
#         return "<V: %.2f %.2f %.2f>" % (self.x, self.y, self.z) # for debugging


class Color:
    r : float
    g : float
    b : float

    def __init__(self, vr=0.0, vg=0.0, vb=0.0):
        (self.r, self.g, self.b) = (vr, vg, vb)

    def __add__(self, other):
        return Color(self.r+other.r, self.g+other.g, self.b+other.b)
#     def __iadd__(self, other):
#         return Color(self.r+other.r, self.g+other.g, self.b+other.b)
    def __mul__(self, other):
        return Color(self.r*other, self.g*other, self.b*other)
#     def __imul__(self, other):
#         return Color(self.r*other, self.y*other, self.z*other)
    def __str__(self):
        return "%d %d %d" % (int(max(0.0, min(self.r*255.0, 255.0))),
                             int(max(0.0, min(self.g*255.0, 255.0))),
                             int(max(0.0, min(self.b*255.0, 255.0))))
#     def __repr__(self):
#         return "<C: %.2f %.2f %.2f>" % (self.r, self.g, self.b) # for debugging


class Luz:
    posicion : Vector
    color : Color
    tipo : str

    def __init__(self, posicion, color, tipo):
        self.posicion = posicion
        self.color = color
        self.tipo = tipo
#     def __repr__(self):
#         return "<L: %s %s %s>" % (self.posicion, self.color, self.tipo) # for debugging


class Material:
    color : Color
    cDifuso : float
    cEspecular : float
    dEspecular : float
    cReflexion : float
    cTransmitividad : float
    iRefraccion : float

    def __init__(self, color, cDifuso=0.0, cEspecular=0.0, dEspecular=0.0,
                 cReflexion=0.0, cTransmitividad=0.0, iRefraccion=0.0):
        self.color = color
        self.cDifuso = cDifuso
        self.cEspecular = cEspecular
        self.dEspecular = dEspecular
        self.cReflexion = cReflexion
        self.cTransmitividad = cTransmitividad
        self.iRefraccion = iRefraccion
#     def __repr__(self): # for debugging
#         return "<M: %r %.2f %.2f %.2f %.2f %.2f %.2f>" %  (
#             self.color, self.cDifuso, self.cEspecular, self.dEspecular,
#             self.cReflexion, self.cTransmitividad, self.iRefraccion)


class Rayo: # &
    origen : Vector
    direccion : Vector
    disInter : float
    objInter : Optional['Cuerpo'] # &

    def __init__(self, origen, direccion):
        self.origen = origen
        self.direccion = direccion
        self.disInter = MAX_DIST
        self.objInter = None


class Cuerpo:
    tipo : str
    material : int

    def init(self, tipo, material):
        self.tipo = tipo
        self.material = material

    def intersecta(self, r : Rayo) -> bool:
        raise NotImplementedError()

    def getNormal(self, punto : Vector) -> Vector:
        raise NotImplementedError()

class Esfera(Cuerpo):
    posicion : Vector
    radio : float

    def __init__(self, material, posicion, radio):
        self.init('esfera', material)
        self.posicion = posicion
        self.radio = radio

    def intersecta(self, r : Rayo):
        esfera_rayo = self.posicion - r.origen
        v = esfera_rayo.pEscalar(r.direccion)

        if v - self.radio > r.disInter: return False
        distChoque = self.radio*self.radio + v*v - esfera_rayo.x*esfera_rayo.x - \
                     esfera_rayo.y*esfera_rayo.y - esfera_rayo.z*esfera_rayo.z
        if distChoque < 0.0: return False

        distChoque = v - math.sqrt(distChoque)
        if distChoque > r.disInter or distChoque < 0.0: return False

        r.disInter = distChoque
        r.objInter = self
        return True

    def getNormal(self, punto : Vector):
        normal = punto - self.posicion
        return normal.normalizar()

#     def __repr__(self): # for debugging
#         return "<S: %d %s %.2f>" % (self.material, self.posicion, self.radio)


class Plano(Cuerpo):
    normal : Vector
    distancia : float

    def __init__(self, material, normal, distancia):
        self.init('plano', material)
        self.normal = normal
        self.normal.normalizar()
        self.distancia = distancia

    def intersecta(self, r : Rayo):
        v = self.normal.pEscalar(r.direccion)
        if v == 0.0: return False

        distChoque = -(self.normal.pEscalar(r.origen) + self.distancia) / v
        if distChoque < 0.0: return False             # Direccion del rayo negativa
        if distChoque > r.disInter: return False  # No es el mas cercano

        r.disInter = distChoque
        r.objInter = self
        return True

    def getNormal(self, punto : Vector):
        return self.normal

#     def __repr__(self): # for debugging
#         return "<P: %d %s %.2f>" % (self.material, self.normal, self.distancia)


class Scene:
    endline : int
    posCamara : Vector
    lookCamara : Vector
    upCamara : Vector
    anchoGrid : int
    altoGrid : int
    look : Vector
    Vhor : Vector
    Vver : Vector
    Vp : Vector

    def __init__(self, scene_filename):
        lines = [l.split() for l in open(scene_filename).readlines() if l.strip() != '' and l.strip()[0] != "#"]
        self.lObjetos : List[Cuerpo] = []
        self.lLuces : List[Luz] = []
        self.lMateriales : List[Material] = []

        # defaults
        self.imgAncho = 320
        self.imgAlto = 200

        self.profTrazado = 3 # bounces
        self.oversampling = 1  # 1 implica que no hay oversampling
        self.campoVision = 60.0

        self.startline = 0 # Start rendering line
        self.endline = self.imgAlto - 1 # End rendering line

        for line in lines:
            word = line[0]
            line = line[1:]

            if word == "size":
                self.imgAncho = int(line[0])
                self.imgAlto = int(line[1])
                self.endline = self.imgAlto - 1 # End rendering line
            elif word == "nbounces":
                self.profTrazado = int(line[0]) # n. bounces
            elif word == "oversampling":
                self.oversampling = int(line[0])
            elif word == "vision":
                self.campoVision = float(line[0])
            elif word == "renderslice":
                self.startline = max(0, int(line[0])) # Start rendering line
                self.endline = min(self.imgAlto-1, int(line[1])) # End rendering line

            elif word == "posCamara":
                self.posCamara = self.parse_vector(line)
            elif word == "lookCamara":
                self.lookCamara = self.parse_vector(line)
            elif word == "upCamara":
                self.upCamara = self.parse_vector(line)

            elif word == "sphere":
                sph = Esfera( int(line[0]), self.parse_vector(line[1:4]), float(line[-1]) )
                self.lObjetos.append(sph)

            elif word == "plano":
                pl = Plano( int(line[0]), self.parse_vector(line[1:4]), float(line[-1]) )
                self.lObjetos.append(pl)

            elif word == "light":
                light = Luz(self.parse_vector(line[0:3]), self.parse_color(line[3:6]), line[-1])
                self.lLuces.append(light)

            elif word == "material":
                mat = self.parse_material(line)
                self.lMateriales.append(mat)

        # iniciamos el raytracer -------------------------------
        self.anchoGrid = self.imgAncho * self.oversampling
        self.altoGrid = self.imgAlto * self.oversampling

        self.look = self.lookCamara - self.posCamara
        self.Vhor = self.look.pVectorial(self.upCamara)
        self.Vhor.normalizar()

        self.Vver = self.look.pVectorial(self.Vhor)
        self.Vver.normalizar()

        fl = self.anchoGrid / (2 * math.tan((0.5 * self.campoVision) * PI_SOBRE_180))

        Vp = self.look
        Vp.normalizar()
        Vp.x = Vp.x * fl - 0.5 * (self.anchoGrid * self.Vhor.x + self.altoGrid * self.Vver.x)
        Vp.y = Vp.y * fl - 0.5 * (self.anchoGrid * self.Vhor.y + self.altoGrid * self.Vver.y)
        Vp.z = Vp.z * fl - 0.5 * (self.anchoGrid * self.Vhor.z + self.altoGrid * self.Vver.z)
        self.Vp = Vp

    # Auxiliary methods
    def parse_vector(self, line):
        return Vector(float(line[0]), float(line[1]), float(line[2]))
    def parse_color(self, line):
        return Color(float(line[0]), float(line[1]), float(line[2]))
    def parse_material(self, line):
        f = [float(x) for x in line[3:]]
        return Material(self.parse_color(line[0:3]), f[0], f[1], f[2], f[3], f[4], f[5])

scene_namefile = 'testdata/scene.txt'
scene = Scene(scene_namefile)

# ----------------- Calcula la sombra de un rayo ------------
def calculaSombra(r : Rayo, objChoque):
    sombra = 1.0   # Incialmente no hay sombra
    for obj in scene.lObjetos:
        r.objInter = None
        r.disInter = MAX_DIST

        if obj.intersecta(r) and obj is not objChoque:
            sombra *= scene.lMateriales[obj.material].cTransmitividad

    return sombra


def trazar(r : Rayo, prof) -> Color:
    c = Color()

    for obj in scene.lObjetos:    # Probamos con todos los objetos
        obj.intersecta(r)

    if r.objInter is not None:
        matIndex = r.objInter.material
        pInterseccion = r.origen + r.direccion * r.disInter
        vIncidente = pInterseccion - r.origen
        vVueltaOrigen = r.direccion * -1.0
        vVueltaOrigen.normalizar()
        vNormal = r.objInter.getNormal(pInterseccion)
        for luz in scene.lLuces:
            if luz.tipo == 'ambiental':
                c += luz.color
            elif luz.tipo == 'puntual':
                dirLuz = luz.posicion - pInterseccion
                dirLuz.normalizar()
                rayoLuz = Rayo(pInterseccion, dirLuz)
                sombra = calculaSombra(rayoLuz, r.objInter)
                NL = vNormal.pEscalar(dirLuz)
                if NL > 0.0:
                    if scene.lMateriales[matIndex].cDifuso > 0.0:  # ------- Difuso
                        colorDifuso = luz.color * scene.lMateriales[matIndex].cDifuso * NL
                        colorDifuso.r *= scene.lMateriales[matIndex].color.r * sombra
                        colorDifuso.g *= scene.lMateriales[matIndex].color.g * sombra
                        colorDifuso.b *= scene.lMateriales[matIndex].color.b * sombra
                        c += colorDifuso
                    if scene.lMateriales[matIndex].cEspecular > 0.0: # ----- Especular
                        rr = (vNormal * 2 * NL) - dirLuz
                        espec = vVueltaOrigen.pEscalar(rr)
                        if espec > 0.0:
                            espec = scene.lMateriales[matIndex].cEspecular * \
                                    math.pow(espec, scene.lMateriales[matIndex].dEspecular)
                            colorEspecular = luz.color * espec * sombra
                            c += colorEspecular
        if prof < scene.profTrazado:
            if scene.lMateriales[matIndex].cReflexion > 0.0:   # -------- Reflexion
                t = vVueltaOrigen.pEscalar(vNormal)
                if t > 0.0:
                    vDirRef = (vNormal * 2 * t) - vVueltaOrigen
                    vOffsetInter = pInterseccion + vDirRef * PEQUENO
                    rayoRef = Rayo(vOffsetInter, vDirRef)
                    c += trazar (rayoRef, prof+1.0) * scene.lMateriales[matIndex].cReflexion
            if scene.lMateriales[matIndex].cTransmitividad > 0.0:  # ---- Refraccion
                RN = vNormal.pEscalar(vIncidente * -1.0)
                n1 : float
                n2 : float
                vIncidente.normalizar()
                if vNormal.pEscalar(vIncidente) > 0.0:
                    vNormal = vNormal * -1.0
                    RN = -RN
                    n1 = scene.lMateriales[matIndex].iRefraccion
                    n2 = 1.0
                else:
                    n2 = scene.lMateriales[matIndex].iRefraccion
                    n1 = 1.0
                if n1 != 0.0 and n2 != 0.0:
                    par_sqrt = math.sqrt(1 - (n1*n1/n2*n2)*(1-RN*RN))
                    vDirRefrac = vIncidente + (vNormal * RN) * (n1/n2) - (vNormal * par_sqrt)
                    vOffsetInter = pInterseccion + vDirRefrac * PEQUENO
                    rayoRefrac = Rayo(vOffsetInter, vDirRefrac)
                    c += trazar(rayoRefrac, prof+1.0) * scene.lMateriales[matIndex].cTransmitividad
    return c

def renderPixel(x, y):
    c = Color()
    x *= scene.oversampling
    y *= scene.oversampling

    for i in range(scene.oversampling):
        for j in range(scene.oversampling):
            direc = Vector()
            direc.x = x * scene.Vhor.x + y * scene.Vver.x + scene.Vp.x
            direc.y = x * scene.Vhor.y + y * scene.Vver.y + scene.Vp.y
            direc.z = x * scene.Vhor.z + y * scene.Vver.z + scene.Vp.z
            direc.normalizar()
            r = Rayo(scene.posCamara, direc)

            c += trazar(r, 1.0)
            y += 1
        x += 1
    srq_oversampling = scene.oversampling * scene.oversampling
    c.r /= srq_oversampling
    c.g /= srq_oversampling
    c.b /= srq_oversampling
    return c

if __name__ == '__main__':
    print("Rendering: " + scene_namefile)

    fileout = open(scene_namefile+".ppm", "w", newline = "\n")
    fileout.write("P3\n")
    fileout.write(str(scene.imgAncho) + ' ' + str(scene.endline - scene.startline + 1) + "\n")
    fileout.write("255\n")

    print("Line (from %d to %d):" % (scene.startline, scene.endline), end=' ')
    for y in range(scene.startline, scene.endline+1):
        for x in range(scene.imgAncho):
            fileout.write(str(renderPixel(x, y)) + ' ')
        fileout.write("\n")
        print(y, end=' ')
        sys.stdout.flush()

    #fileout.close()
