#include <karm-base/clamp.h>
#include <karm-base/rc.h>
#include <karm-base/var.h>
#include <karm-base/vec.h>
#include <karm-debug/logger.h>
#include <karm-gfx/buffer.h>
#include <karm-main/main.h>
#include <karm-math/vec.h>
#include <karm-ui/app.h>
#include <karm-ui/layout.h>
#include <karm-ui/view.h>

using namespace Math;

using Colorf = Vec3f;
struct Object;

struct Props {
    int samples = 1;
    int bounces = 1;
};

struct Ray3f {
    Vec3f o;
    Vec3f dir;
    double tMin = 0.001;
    double tMax = INFINITY;

    constexpr Vec3f at(double t) const {
        return o + dir * t;
    }
};

struct Hit {
    Vec3f in;
    Vec3f pos;
    Vec3f normal;
    double t;
    Object *what = nullptr;
};

struct Sample {
    Vec3f out{};
    Colorf att{1};
    Colorf emit{0};
    double pdf{};
};

double randf() {
    return (double)rand() / (double)RAND_MAX;
}

double randf(double min, double max) {
    return min + (max - min) * randf();
}

Vec3f randomInUnitSphere() {
    return Vec3f{randf(-1, 1), randf(-1, 1), randf(-1, 1)}.norm();
}

Vec3f randomInHemisphere(Vec3f normal) {
    Vec3f inUnitSphere = randomInUnitSphere();
    if (inUnitSphere.dot(normal) > 0.0) {
        return inUnitSphere;
    } else {
        return -inUnitSphere;
    }
}

Vec3f reflect(Vec3f ray, Vec3f surface) {
    return ray - 2 * ray.dot(surface) * surface;
}

/* --- Geometry ------------------------------------------------------------- */

struct Sphere {
    Vec3f center;
    double size;
};

Opt<Hit> hit(Sphere const &sphere, Ray3f ray) {
    Vec3f oc = ray.o - sphere.center;
    auto a = ray.dir.lenSq();
    auto halfB = oc.dot(ray.dir);
    auto c = oc.lenSq() - sphere.size * sphere.size;
    auto d = halfB * halfB - a * c;

    if (d < 0) {
        return NONE;
    }

    auto t = (-halfB - sqrt(d)) / a;

    if (t < ray.tMin || t > ray.tMax) {
        t = (-halfB + sqrt(d)) / a;
        if (t < ray.tMin || t > ray.tMax) {
            return NONE;
        }
    }

    auto p = ray.at(t);

    return Hit{
        ray.dir,
        p,
        (p - sphere.center) / sphere.size,
        t,
    };
}

struct Plane {
    Vec3f pos;
    Vec3f normal;
    Vec2f size;
};

Opt<Hit> hit(Plane const &plane, Ray3f ray) {
    auto denom = plane.normal.dot(ray.dir);

    if (denom > 1e-6) {
        auto t = (plane.pos - ray.o).dot(plane.normal) / denom;
        if (t >= ray.tMin && t <= ray.tMax) {
            auto p = ray.at(t);
            auto pp = p - plane.normal * plane.normal.dot(p - plane.pos);
            auto uv = (pp - plane.pos).xy / plane.size;

            if (uv.x >= -1 && uv.x <= 1 &&
                uv.y >= -1 && uv.y <= 1) {
                return Hit{
                    ray.dir,
                    p,
                    plane.normal,
                    t,
                };
            }
        }
    }

    return NONE;
}

struct Geometry : public Var<Sphere, Plane> {
    using Var::Var;
};

Opt<Hit> hit(Geometry const &geometry, Ray3f ray) {
    return geometry.visit([&](auto &g) {
        return hit(g, ray);
    });
}

/* --- Material ------------------------------------------------------------- */

struct Material;

Colorf sample(Colorf &albedo) {
    return albedo;
}

struct Perlin {
    Vec2f scale;
    double octaves;
    double persistence;
};

Colorf sample(Perlin &) {
    return Colorf{1, 0, 1};
}

struct Image {
};

Colorf sample(Image &) {
    return Colorf{1, 1, 0};
}

using Texture = Var<Colorf, Perlin, Image>;

Colorf sample(Texture texture) {
    return texture.visit([&](auto &t) {
        return sample(t);
    });
}

struct Roughness {
    Texture albedo;
    double roughness = 0.0;
};

Sample eval(Roughness &mat, Hit hit) {
    auto reflected = reflect(hit.in, hit.normal);

    return {
        //.out = (reflected + randomInHemisphere(hit.normal) * mat.roughness).norm(),
        .out = (reflected + randomInUnitSphere() * mat.roughness).norm(),
        .att = sample(mat.albedo),
    };
}

struct Emmisive {
    Texture albedo;
};

Sample eval(Emmisive &mat, Hit hit) {
    return {
        .out = hit.normal + randomInUnitSphere(),
        .emit = sample(mat.albedo),
    };
}

struct Isotropic {
    Texture albedo;
};

Sample eval(Isotropic &isotropic, Hit) {
    return {
        .out = randomInUnitSphere(),
        .att = sample(isotropic.albedo),
    };
}

struct Material : Var<Roughness, Emmisive, Isotropic> {
    using Var::Var;
};

Sample eval(Material &material, Hit hit) {
    return material.visit([&](auto &m) {
        return eval(m, hit);
    });
}

/* --- Scene ---------------------------------------------------------------- */

enum struct ObjectFlags : uint32_t {
    NONE = 0,
    INVISIBLE = 1 << 0,
};

FlagsEnum$(ObjectFlags);

struct Object {
    ObjectFlags flags;
    Geometry geometry;
    Material material;
};

Opt<Hit> hit(Object const &object, Ray3f ray) {
    auto hit = ::hit(object.geometry, ray);
    if (hit) {
        hit->what = (Object *)&object;
    }
    return hit;
}

using Scene = Vec<Object>;

Opt<Hit> hit(Scene &scene, Ray3f ray, Props props, int depth) {
    Opt<Hit> closest;
    for (auto &object : scene) {
        auto hit = ::hit(object, ray);
        if (hit && (!closest || hit->t < closest->t)) {
            bool invisible = (bool)(object.flags & ObjectFlags::INVISIBLE);
            if (depth == props.bounces && invisible) {
                continue;
            }
            closest = hit;
            ray.tMax = hit->t;
        }
    }
    return closest;
}

/* --- Camera --------------------------------------------------------------- */

struct Cam {
    Math::Vec3f pos;
    Math::Vec3f dir;
    double focal;
    double ratio;
};

[[gnu::flatten]] Colorf castRay(Ray3f ray, Scene &scene, Props props, int depth) {
    if (depth == 0) {
        return 0;
    }

    auto maybeHit = hit(scene, ray, props, depth);
    if (maybeHit) {
        auto hit = *maybeHit;
        auto *obj = hit.what;
        auto sample = eval(obj->material, hit);
        Ray3f newRay = Ray3f{hit.pos, (sample.out).norm()};

        return sample.emit + (sample.att * castRay(newRay, scene, props, depth - 1));
    }

    return 0;

    Vec3f unitDir = ray.dir.norm();
    double t = 0.5 * (unitDir.y + 1.0);
    return (1.0 - t) * Colorf(1.0) + t * Colorf(0.5, 0.7, 1.0);
}

Colorf acesTonemap(Colorf color) {
    double a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return (color * (a * color + b)) / (color * (c * color + d) + e);
}

void renderScene(Cam cam, Scene &scene, Gfx::Surface buf, Props props) {
    auto size = buf.bound().size();

    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            // NOTE: size.y - y  because the image is flipped
            auto uv = (Vec2i{x, size.y - y} / size.cast<double>() - Vec2f{0.5, 0.5});

            Colorf colorf(0.0);
            for (int i = 0; i < props.samples; i++) {
                auto uuvv = uv + Vec2f{randf(), randf()} / size.cast<double>();
                auto dir = Vec3f{uuvv.x * cam.ratio, uuvv.y, -cam.focal}.norm();
                auto ray = Ray3f{cam.pos, dir};

                colorf = colorf + castRay(ray, scene, props, props.bounces);
            }

            // gamma correction
            auto scale = 1.0 / props.samples;
            colorf = Colorf{sqrt(scale * colorf.x),
                            sqrt(scale * colorf.y),
                            sqrt(scale * colorf.z)};

            colorf = acesTonemap(colorf);

            auto color = Gfx::Color::fromRgb(
                clamp01(colorf.x) * 255,
                clamp01(colorf.y) * 255,
                clamp01(colorf.z) * 255);

            buf.store({x, y}, color);
        }

        if (y % 10 == 0)
            Debug::linfo("rendered {} of {}", y, size.y);
    }
}

CliResult entryPoint(CliArgs args) {
    srand(1);
    double scale = 1;

    Media::Image image{Gfx::RGBA8888, Vec2f{1280 * scale, 720 * scale}.cast<int>()};

    Props props = {
        .samples = 10,
        .bounces = 4,
    };

    Cam cam{
        .pos = {0, 0, 1},
        .dir = {0, 0, 1},
        .focal = 1,
        .ratio = 16.0 / 9.0,
    };

    Scene scene = {
        Object{
            ObjectFlags::NONE,
            Sphere{{0, 0, -1}, 0.5},
            Roughness{Colorf{0.5, 0.5, 0.5}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{0, 0, -2}, {0, 0, -1}, 1},
            Roughness{Colorf{1, 1, 1}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{-1, 0, -2}, {-1, 0, 0}, 1},
            Roughness{Colorf{1, 0, 0}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{1, 0, -2}, {1, 0, 0}, 1},
            Roughness{Colorf{0, 1, 0}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{0, -1, -2}, {0, -1, 0}, 1},
            Roughness{Colorf{1}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{0, 1, -2}, {0, 1, 0}, 1},
            Roughness{Colorf{1}, 1},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{0, 0.9, -1}, {0, 1, 0}, 0.25},
            Emmisive{Colorf{1, 1, 1}},
        },

        Object{
            ObjectFlags::NONE,
            Plane{{0, 1, -1}, {0, 1, 0}, 0.25},
            Emmisive{Colorf{1, 1, 1}},
        },

        /*
                Object{
                    ObjectFlags::NONE,
                    Sphere{{0, -100.5, -1}, 100},
                    Roughness{Colorf{0.5, 0.5, 0.5}, 1},
                },
                */
    };

    renderScene(cam, scene, image, props);

    return Ui::runApp(args, Ui::minSize({1920, 1080}, Ui::image(image)));
}
