#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto MAX_DIST = 1999999999.0;
auto PI_SOBRE_180 = 0.017453292;
auto PEQUENO = 0.000000001;

template <typename T1> auto colorToString(const T1 &c)
{
    return u"#. #. #."_S.format(to_int(max(0.0, min(c.r * 255.0, 255.0))), to_int(max(0.0, min(c.g * 255.0, 255.0))), to_int(max(0.0, min(c.b * 255.0, 255.0))));
}

class Luz
{
public:
    dvec3 posicion;
    dvec3 color;
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
    dvec3 color;
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
    dvec3 origen;
    dvec3 direccion;
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
    virtual dvec3 getNormal(const dvec3 &punto) = 0;
};

class Esfera : public Cuerpo
{
public:
    dvec3 posicion;
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
        auto v = dot(esfera_rayo, r.direccion);
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

    virtual dvec3 getNormal(const dvec3 &punto) override
    {
        auto normal = punto - posicion;
        return normalize(normal);
    }
};

class Plano : public Cuerpo
{
public:
    dvec3 normal;
    double distancia;

    template <typename T1, typename T2, typename T3> Plano(const T1 &material, const T2 &normal, const T3 &distancia) :
        distancia(distancia)
    {
        init(u"plano"_S, material);
        this->normal = normalize(normal);
    }

    virtual bool intersecta(Rayo &r) override
    {
        auto v = dot(normal, r.direccion);
        if (v == 0.0)
            return false;
        auto distChoque = -(dot(normal, r.origen) + distancia) / v;
        if (distChoque < 0.0)
            return false;
        if (distChoque > r.disInter)
            return false;
        r.disInter = distChoque;
        r.objInter = &*this;
        return true;
    }

    virtual dvec3 getNormal(const dvec3 &punto) override
    {
        return normal;
    }
};

class Scene
{
public:
    int endline;
    dvec3 posCamara;
    dvec3 lookCamara;
    dvec3 upCamara;
    int anchoGrid;
    int altoGrid;
    dvec3 look;
    dvec3 Vhor;
    dvec3 Vver;
    dvec3 Vp;
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
        Vhor = normalize(cross(upCamara, look));
        Vver = normalize(cross(Vhor, look));
        auto fl = anchoGrid / (2 * tan((0.5 * campoVision) * ::PI_SOBRE_180));
        auto Vp = normalize(look);
        Vp.x = Vp.x * fl - 0.5 * (anchoGrid * Vhor.x + altoGrid * Vver.x);
        Vp.y = Vp.y * fl - 0.5 * (anchoGrid * Vhor.y + altoGrid * Vver.y);
        Vp.z = Vp.z * fl - 0.5 * (anchoGrid * Vhor.z + altoGrid * Vver.z);
        this->Vp = Vp;
    }

    template <typename T1> auto parse_vector(const T1 &line)
    {
        return make_tuple(to_float(_get<0>(line)), to_float(_get<1>(line)), to_float(_get<2>(line)));
    }
    template <typename T1> auto parse_color(const T1 &line)
    {
        return make_tuple(to_float(_get<0>(line)), to_float(_get<1>(line)), to_float(_get<2>(line)));
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

template <typename T2> dvec3 trazar(Rayo &r, const T2 &prof)
{
    auto c = make_tuple(0.0, 0.0, 0.0);
    for (auto &&obj : ::scene.lObjetos)
        obj->intersecta(r);

    if (r.objInter != nullptr) {
        auto matIndex = r.objInter->material;
        auto pInterseccion = r.origen + r.direccion * r.disInter;
        auto vIncidente = pInterseccion - r.origen;
        auto vVueltaOrigen = normalize(r.direccion * -1.0);
        auto vNormal = r.objInter->getNormal(pInterseccion);
        for (auto &&luz : ::scene.lLuces)
            if (luz.tipo == u"ambiental")
                c += luz.color;
            else if (luz.tipo == u"puntual") {
                auto dirLuz = normalize(luz.posicion - pInterseccion);
                auto rayoLuz = Rayo(pInterseccion, dirLuz);
                auto sombra = calculaSombra(rayoLuz, r.objInter);
                auto NL = dot(vNormal, dirLuz);
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
                        auto espec = dot(vVueltaOrigen, rr);
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
                auto t = dot(vVueltaOrigen, vNormal);
                if (t > 0.0) {
                    auto vDirRef = (vNormal * 2 * t) - vVueltaOrigen;
                    auto vOffsetInter = pInterseccion + vDirRef * ::PEQUENO;
                    auto rayoRef = Rayo(vOffsetInter, vDirRef);
                    c += trazar(rayoRef, prof + 1.0) * ::scene.lMateriales[matIndex].cReflexion;
                }
            }
            if (::scene.lMateriales[matIndex].cTransmitividad > 0.0) {
                auto RN = dot(vNormal, vIncidente * -1.0);
                double n1;
                double n2;
                vIncidente = normalize(vIncidente);
                if (dot(vNormal, vIncidente) > 0.0) {
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
    auto c = make_tuple(0.0, 0.0, 0.0);
    x *= ::scene.oversampling;
    y *= ::scene.oversampling;

    for (auto i : range_el(0, ::scene.oversampling)) {
        for (auto j : range_el(0, ::scene.oversampling)) {
            dvec3 direc;
            direc.x = x * ::scene.Vhor.x + y * ::scene.Vver.x + ::scene.Vp.x;
            direc.y = x * ::scene.Vhor.y + y * ::scene.Vver.y + ::scene.Vp.y;
            direc.z = x * ::scene.Vhor.z + y * ::scene.Vver.z + ::scene.Vp.z;
            auto r = Rayo(::scene.posCamara, normalize(direc));
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
    print(u"Rendering: "_S & scene_namefile);
    auto fileout = File(scene_namefile & u".ppm"_S, u"w"_S);
    fileout.write(u"P3\n"_S);
    fileout.write(String(scene.imgAncho) & u" "_S & String(scene.endline - scene.startline + 1) & u"\n"_S);
    fileout.write(u"255\n"_S);
    print(u"Line (from #.0 to #.0):"_S.format(scene.startline, scene.endline), u" "_S);
    for (auto y : range_el(scene.startline, scene.endline + 1)) {
        for (auto x : range_el(0, scene.imgAncho))
            fileout.write(colorToString(renderPixel(x, y)) & u" "_S);
        fileout.write(u"\n"_S);
        print(y, u" "_S);
        _stdout.flush();
    }
}
