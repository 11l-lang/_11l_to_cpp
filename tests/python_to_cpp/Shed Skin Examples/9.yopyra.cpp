#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto MAX_DIST = 1999999999.0;
auto PI_SOBRE_180 = 0.017453292;
auto PEQUENO = 0.000000001;

class Vector
{
public:
    double x;
    double y;
    double z;

    template <typename T1 = decltype(0.0), typename T2 = decltype(0.0), typename T3 = decltype(0.0)> Vector(const T1 &vx = 0.0, const T2 &vy = 0.0, const T3 &vz = 0.0)
    {
        assign_from_tuple(x, y, z, make_tuple(vx, vy, vz));
    }

    template <typename T1> auto pEscalar(const T1 &vv)
    {
        return (x * vv.x + y * vv.y + z * vv.z);
    }
    template <typename T1> auto pVectorial(const T1 &vv)
    {
        auto r = Vector();
        r.x = vv.y * z - vv.z * y;
        r.y = vv.z * x - vv.x * z;
        r.z = vv.x * y - vv.y * x;
        return r;
    }
    auto modulo()
    {
        return sqrt(x * x + y * y + z * z);
    }
    auto normalizar()
    {
        auto m = modulo();
        if (m != 0.0) {
            x /= m;
            y /= m;
            z /= m;
        }
        return *this;
    }

    template <typename T1> auto operator+(const T1 &other) const
    {
        return Vector(x + other.x, y + other.y, z + other.z);
    }
    template <typename Ty> auto &operator+=(const Ty &t)
    {
        *this = *this + t;
        return *this;
    }
    template <typename T1> auto operator-(const T1 &other) const
    {
        return Vector(x - other.x, y - other.y, z - other.z);
    }
    template <typename Ty> auto &operator-=(const Ty &t)
    {
        *this = *this - t;
        return *this;
    }
    template <typename T1> auto operator*(const T1 &other) const
    {
        return Vector(x * other, y * other, z * other);
    }
    template <typename Ty> auto &operator*=(const Ty &t)
    {
        *this = *this * t;
        return *this;
    }
};

class Color
{
public:
    double r;
    double g;
    double b;

    template <typename T1 = decltype(0.0), typename T2 = decltype(0.0), typename T3 = decltype(0.0)> Color(const T1 &vr = 0.0, const T2 &vg = 0.0, const T3 &vb = 0.0)
    {
        assign_from_tuple(r, g, b, make_tuple(vr, vg, vb));
    }

    template <typename T1> auto operator+(const T1 &other) const
    {
        return Color(r + other.r, g + other.g, b + other.b);
    }
    template <typename Ty> auto &operator+=(const Ty &t)
    {
        *this = *this + t;
        return *this;
    }

    template <typename T1> auto operator*(const T1 &other) const
    {
        return Color(r * other, g * other, b * other);
    }
    template <typename Ty> auto &operator*=(const Ty &t)
    {
        *this = *this * t;
        return *this;
    }

    operator String()
    {
        return u"#.0 #.0 #.0"_S.format(to_int(max(0.0, min(r * 255.0, 255.0))), to_int(max(0.0, min(g * 255.0, 255.0))), to_int(max(0.0, min(b * 255.0, 255.0))));
    }
};

class Luz
{
public:
    Vector posicion;
    Color color;
    String tipo;

    template <typename T1, typename T2, typename T3> Luz(const T1 &posicion, const T2 &color, const T3 &tipo) :
        posicion(posicion),
        color(color),
        tipo(tipo)
    {
    }
};

class Material
{
public:
    Color color;
    double cDifuso;
    double cEspecular;
    double dEspecular;
    double cReflexion;
    double cTransmitividad;
    double iRefraccion;

    template <typename T1, typename T2 = decltype(0.0), typename T3 = decltype(0.0), typename T4 = decltype(0.0), typename T5 = decltype(0.0), typename T6 = decltype(0.0), typename T7 = decltype(0.0)> Material(const T1 &color, const T2 &cDifuso = 0.0, const T3 &cEspecular = 0.0, const T4 &dEspecular = 0.0, const T5 &cReflexion = 0.0, const T6 &cTransmitividad = 0.0, const T7 &iRefraccion = 0.0) :
        color(color),
        cDifuso(cDifuso),
        cEspecular(cEspecular),
        dEspecular(dEspecular),
        cReflexion(cReflexion),
        cTransmitividad(cTransmitividad),
        iRefraccion(iRefraccion)
    {
    }
};
class Cuerpo;


class Rayo
{
public:
    Vector origen;
    Vector direccion;
    double disInter;
    Cuerpo *objInter;

    template <typename T1, typename T2> Rayo(const T1 &origen, const T2 &direccion) :
        origen(origen),
        direccion(direccion)
    {
        disInter = ::MAX_DIST;
        objInter = nullptr;
    }
};

class Cuerpo
{
public:
    String tipo;
    int material;

    template <typename T1, typename T2> auto init(const T1 &tipo, const T2 &material)
    {
        this->tipo = tipo;
        this->material = material;
    }
    virtual bool intersecta(Rayo &r) = 0;
    virtual Vector getNormal(const Vector &punto) = 0;
};

class Esfera : public Cuerpo
{
public:
    Vector posicion;
    double radio;

    template <typename T1, typename T2, typename T3> Esfera(const T1 &material, const T2 &posicion, const T3 &radio) :
        posicion(posicion),
        radio(radio)
    {
        init(u"esfera"_S, material);
    }

    virtual bool intersecta(Rayo &r) override
    {
        auto esfera_rayo = posicion - r.origen;
        auto v = esfera_rayo.pEscalar(r.direccion);
        if (v - radio > r.disInter)
            return false;
        auto distChoque = radio * radio + v * v - esfera_rayo.x * esfera_rayo.x - esfera_rayo.y * esfera_rayo.y - esfera_rayo.z * esfera_rayo.z;
        if (distChoque < 0.0)
            return false;
        distChoque = v - sqrt(distChoque);
        if (distChoque > r.disInter || distChoque < 0.0)
            return false;
        r.disInter = distChoque;
        r.objInter = &*this;
        return true;
    }

    virtual Vector getNormal(const Vector &punto) override
    {
        auto normal = punto - posicion;
        return normal.normalizar();
    }
};

class Plano : public Cuerpo
{
public:
    Vector normal;
    double distancia;

    template <typename T1, typename T2, typename T3> Plano(const T1 &material, const T2 &normal, const T3 &distancia) :
        normal(normal),
        distancia(distancia)
    {
        init(u"plano"_S, material);
        this->normal.normalizar();
    }

    virtual bool intersecta(Rayo &r) override
    {
        auto v = normal.pEscalar(r.direccion);
        if (v == 0.0)
            return false;
        auto distChoque = -(normal.pEscalar(r.origen) + distancia) / v;
        if (distChoque < 0.0)
            return false;
        if (distChoque > r.disInter)
            return false;
        r.disInter = distChoque;
        r.objInter = &*this;
        return true;
    }

    virtual Vector getNormal(const Vector &punto) override
    {
        return normal;
    }
};

class Scene
{
public:
    int endline;
    Vector posCamara;
    Vector lookCamara;
    Vector upCamara;
    int anchoGrid;
    int altoGrid;
    Vector look;
    Vector Vhor;
    Vector Vver;
    Vector Vp;
    Array<std::unique_ptr<Cuerpo>> lObjetos;
    Array<Luz> lLuces;
    Array<Material> lMateriales;
    decltype(320) imgAncho = 320;
    decltype(200) imgAlto = 200;
    decltype(3) profTrazado = 3;
    decltype(1) oversampling = 1;
    decltype(60.0) campoVision = 60.0;
    decltype(0) startline = 0;

    template <typename T1> Scene(const T1 &scene_filename)
    {
        auto lines = File(scene_filename).read_lines(true).filter([](const auto &l){return l.trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S)) != u"" && _get<0>(l.trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S))) != u'#';}).map([](const auto &l){return l.split_py();});
        endline = imgAlto - 1;

        for (auto &&line : lines) {
            auto word = _get<0>(line);
            line = line[range_ei(1)];

            if (word == u"size") {
                imgAncho = to_int(_get<0>(line));
                imgAlto = to_int(_get<1>(line));
                endline = imgAlto - 1;
            }
            else if (word == u"nbounces")
                profTrazado = to_int(_get<0>(line));
            else if (word == u"oversampling")
                oversampling = to_int(_get<0>(line));
            else if (word == u"vision")
                campoVision = to_float(_get<0>(line));
            else if (word == u"renderslice") {
                startline = max(0, to_int(_get<0>(line)));
                endline = min(imgAlto - 1, to_int(_get<1>(line)));
            }
            else if (word == u"posCamara")
                posCamara = parse_vector(line);
            else if (word == u"lookCamara")
                lookCamara = parse_vector(line);
            else if (word == u"upCamara")
                upCamara = parse_vector(line);

            else if (word == u"sphere") {
                auto sph = std::make_unique<Esfera>(to_int(_get<0>(line)), parse_vector(line[range_el(1, 4)]), to_float(line.last()));
                lObjetos.append(std::move(sph));
            }

            else if (word == u"plano") {
                auto pl = std::make_unique<Plano>(to_int(_get<0>(line)), parse_vector(line[range_el(1, 4)]), to_float(line.last()));
                lObjetos.append(std::move(pl));
            }

            else if (word == u"light") {
                auto light = Luz(parse_vector(line[range_el(0, 3)]), parse_color(line[range_el(3, 6)]), line.last());
                lLuces.append(light);
            }

            else if (word == u"material") {
                auto mat = parse_material(line);
                lMateriales.append(mat);
            }
        }
        anchoGrid = imgAncho * oversampling;
        altoGrid = imgAlto * oversampling;
        look = lookCamara - posCamara;
        Vhor = look.pVectorial(upCamara);
        Vhor.normalizar();
        Vver = look.pVectorial(Vhor);
        Vver.normalizar();
        auto fl = anchoGrid / (2 * tan((0.5 * campoVision) * ::PI_SOBRE_180));
        auto Vp = look;
        Vp.normalizar();
        Vp.x = Vp.x * fl - 0.5 * (anchoGrid * Vhor.x + altoGrid * Vver.x);
        Vp.y = Vp.y * fl - 0.5 * (anchoGrid * Vhor.y + altoGrid * Vver.y);
        Vp.z = Vp.z * fl - 0.5 * (anchoGrid * Vhor.z + altoGrid * Vver.z);
        this->Vp = Vp;
    }

    template <typename T1> auto parse_vector(const T1 &line)
    {
        return Vector(to_float(_get<0>(line)), to_float(_get<1>(line)), to_float(_get<2>(line)));
    }
    template <typename T1> auto parse_color(const T1 &line)
    {
        return Color(to_float(_get<0>(line)), to_float(_get<1>(line)), to_float(_get<2>(line)));
    }
    template <typename T1> auto parse_material(const T1 &line)
    {
        auto f = line[range_ei(3)].map([](const auto &x){return to_float(x);});
        return Material(parse_color(line[range_el(0, 3)]), _get<0>(f), _get<1>(f), _get<2>(f), _get<3>(f), _get<4>(f), _get<5>(f));
    }
};
auto scene_namefile = u"testdata/scene.txt"_S;
auto scene = Scene(scene_namefile);

template <typename T2> auto calculaSombra(Rayo &r, const T2 &objChoque)
{
    auto sombra = 1.0;
    for (auto &&obj : ::scene.lObjetos) {
        r.objInter = nullptr;
        r.disInter = ::MAX_DIST;
        if (obj->intersecta(r) && &*obj != objChoque)
            sombra *= ::scene.lMateriales[obj->material].cTransmitividad;
    }
    return sombra;
}

template <typename T2> Color trazar(Rayo &r, const T2 &prof)
{
    auto c = Color();
    for (auto &&obj : ::scene.lObjetos)
        obj->intersecta(r);

    if (r.objInter != nullptr) {
        auto matIndex = r.objInter->material;
        auto pInterseccion = r.origen + r.direccion * r.disInter;
        auto vIncidente = pInterseccion - r.origen;
        auto vVueltaOrigen = r.direccion * -1.0;
        vVueltaOrigen.normalizar();
        auto vNormal = r.objInter->getNormal(pInterseccion);
        for (auto &&luz : ::scene.lLuces)
            if (luz.tipo == u"ambiental")
                c += luz.color;
            else if (luz.tipo == u"puntual") {
                auto dirLuz = luz.posicion - pInterseccion;
                dirLuz.normalizar();
                auto rayoLuz = Rayo(pInterseccion, dirLuz);
                auto sombra = calculaSombra(rayoLuz, r.objInter);
                auto NL = vNormal.pEscalar(dirLuz);
                if (NL > 0.0) {
                    if (::scene.lMateriales[matIndex].cDifuso > 0.0) {
                        auto colorDifuso = luz.color * ::scene.lMateriales[matIndex].cDifuso * NL;
                        colorDifuso.r *= ::scene.lMateriales[matIndex].color.r * sombra;
                        colorDifuso.g *= ::scene.lMateriales[matIndex].color.g * sombra;
                        colorDifuso.b *= ::scene.lMateriales[matIndex].color.b * sombra;
                        c += colorDifuso;
                    }
                    if (::scene.lMateriales[matIndex].cEspecular > 0.0) {
                        auto rr = (vNormal * 2 * NL) - dirLuz;
                        auto espec = vVueltaOrigen.pEscalar(rr);
                        if (espec > 0.0) {
                            espec = ::scene.lMateriales[matIndex].cEspecular * pow(espec, ::scene.lMateriales[matIndex].dEspecular);
                            auto colorEspecular = luz.color * espec * sombra;
                            c += colorEspecular;
                        }
                    }
                }
            }
        if (prof < ::scene.profTrazado) {
            if (::scene.lMateriales[matIndex].cReflexion > 0.0) {
                auto t = vVueltaOrigen.pEscalar(vNormal);
                if (t > 0.0) {
                    auto vDirRef = (vNormal * 2 * t) - vVueltaOrigen;
                    auto vOffsetInter = pInterseccion + vDirRef * ::PEQUENO;
                    auto rayoRef = Rayo(vOffsetInter, vDirRef);
                    c += trazar(rayoRef, prof + 1.0) * ::scene.lMateriales[matIndex].cReflexion;
                }
            }
            if (::scene.lMateriales[matIndex].cTransmitividad > 0.0) {
                auto RN = vNormal.pEscalar(vIncidente * -1.0);
                double n1;
                double n2;
                vIncidente.normalizar();
                if (vNormal.pEscalar(vIncidente) > 0.0) {
                    vNormal = vNormal * -1.0;
                    RN = -RN;
                    n1 = ::scene.lMateriales[matIndex].iRefraccion;
                    n2 = 1.0;
                }
                else {
                    n2 = ::scene.lMateriales[matIndex].iRefraccion;
                    n1 = 1.0;
                }
                if (n1 != 0.0 && n2 != 0.0) {
                    auto par_sqrt = sqrt(1 - (n1 * n1 / n2 * n2) * (1 - RN * RN));
                    auto vDirRefrac = vIncidente + (vNormal * RN) * (n1 / n2) - (vNormal * par_sqrt);
                    auto vOffsetInter = pInterseccion + vDirRefrac * ::PEQUENO;
                    auto rayoRefrac = Rayo(vOffsetInter, vDirRefrac);
                    c += trazar(rayoRefrac, prof + 1.0) * ::scene.lMateriales[matIndex].cTransmitividad;
                }
            }
        }
    }
    return c;
}

template <typename T1, typename T2> auto renderPixel(T1 x, T2 y)
{
    auto c = Color();
    x *= ::scene.oversampling;
    y *= ::scene.oversampling;

    for (auto i : range_el(0, ::scene.oversampling)) {
        for (auto j : range_el(0, ::scene.oversampling)) {
            auto direc = Vector();
            direc.x = x * ::scene.Vhor.x + y * ::scene.Vver.x + ::scene.Vp.x;
            direc.y = x * ::scene.Vhor.y + y * ::scene.Vver.y + ::scene.Vp.y;
            direc.z = x * ::scene.Vhor.z + y * ::scene.Vver.z + ::scene.Vp.z;
            direc.normalizar();
            auto r = Rayo(::scene.posCamara, direc);
            c += trazar(r, 1.0);
            y++;
        }
        x++;
    }
    auto srq_oversampling = ::scene.oversampling * ::scene.oversampling;
    c.r /= srq_oversampling;
    c.g /= srq_oversampling;
    c.b /= srq_oversampling;
    return c;
}

int main()
{
    print(u"Rendering: "_S + scene_namefile);
    auto fileout = File(scene_namefile + u".ppm"_S, u"w"_S);
    fileout.write(u"P3\n"_S);
    fileout.write(String(scene.imgAncho) + u" "_S + String(scene.endline - scene.startline + 1) + u"\n"_S);
    fileout.write(u"255\n"_S);
    print(u"Line (from #.0 to #.0):"_S.format(scene.startline, scene.endline), u" "_S);
    for (auto y : range_el(scene.startline, scene.endline + 1)) {
        for (auto x : range_el(0, scene.imgAncho))
            fileout.write(String(renderPixel(x, y)) + u" "_S);
        fileout.write(u"\n"_S);
        print(y, u" "_S);
        _stdout.flush();
    }
}
