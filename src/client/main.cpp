#include <iostream>
#include <vector>
#include "CL/cl.h"
#include "client/clerror.hpp"
#include "client/renderer.hpp"
#include "client/device.hpp"
#include "utils/log.hpp"

namespace log = echidna::log;

auto kernel = R"(
#define EPSILON (1e-4)
#define PI (3.14159265f)
#define TAU (2 * PI)

struct ray {
    float3 o;
    float3 d;
};

enum refl_type {
    REFL_DIFF,
    REFL_SPEC,
    REFL_REFR
};

struct sphere {
    double r;
    float3 p, e, c;
    enum refl_type refl;
};

constant struct sphere scene[] = {
    {1e5, {1e5+1,40.8,81.6}, {}, {.75, .25, .25}, REFL_DIFF},
    {1e5, {-1e5+99,40.8,81.6}, {}, {.25, .25, .75}, REFL_DIFF},
    {1e5, {50, 40.8, 1e5}, {}, {.75, .75, .75}, REFL_DIFF},
    {1e5, {50, 1e5, 81.6}, {}, {.75, .75, .75}, REFL_DIFF},
    {1e5, {50,-1e5 + 81.6, 81.6}, {}, {.75, .75, .75}, REFL_DIFF},
    {1e5, {50,40.8,-1e5 + 300}, {}, {}, REFL_DIFF},
    {16.5, {27, 16.5, 47}, {}, {.999, .999, .999}, REFL_SPEC},
    {16.5, {73, 16.5, 78}, {}, {.999, .999, .999}, REFL_REFR},
    {600, {50, 681.6-.27, 81.6}, {12, 12, 12}, {}, REFL_DIFF},
};

float rand(float* seed) {
    float ipart;
    return fract(sin((*seed)++) * 43758.5453123, &ipart);
}

double sphere_intersect(constant struct sphere* s, struct ray r) {
    float3 p = s->p - r.o;
    float b = dot(p, r.d);
    float d = b * b - dot(p, p) + s->r * s->r;

    if (d < 0)
        return 0;

    d = sqrt(d);
    float t;
    return (t = b - d) > EPSILON ? t : (t = b + d) > EPSILON ? t : 0.f;
}

inline int scene_intersect(struct ray r, float* t, int ignore) {
    int i = -1;
    float t0 = 1e20;
    for (int j = 0; j < sizeof(scene) / sizeof(struct sphere); ++j) {
        float t1 = sphere_intersect(&scene[j], r);
        if (j != ignore && t1 > EPSILON && t1 < t0) {
            i = j;
            t0 = t1;
        }
    }

    *t = t0;
    return i;
}

float3 radiance(struct ray r, int depth, float* seed) {
    int last = -1;
    float3 col = {1, 1, 1};

    for (int i = 0; i < depth; ++i) {
        float t;
        int index = scene_intersect(r, &t, last);
        if (index == -1) {
            return (float3){0, 0, 0};
        }
        last = index;

        constant struct sphere* s = &scene[index];
        float3 x = r.o + r.d * t;
        float3 n = normalize(x - s->p);
        float3 nl = dot(n, r.d) < 0 ? n : -n;
        float3 f = s->c;
        float p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z;
        col *= f;
        col += s->e;

        r.o = x;

        if (s->refl == REFL_DIFF) {
            float r1 = TAU * rand(seed);
            float r2 = rand(seed);
            float r2s = sqrt(r2);
            float3 w = nl;
            float3 u = normalize(fmod(fabs(w.x) > .1 ? (float3){0, 1, 0} : (float3){1, 0, 0}, w));
            float3 v = cross(w, u);
            float3 d = normalize(u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2));
            r.d = d;
        } else if (s->refl == REFL_SPEC) {
            r.d -= n * 2 * dot(n, r.d);
        } else {
            // REFR
            struct ray refl_ray = {x, r.d * n * 2 * dot(n, r.d)};
            bool into = dot(n, nl) > 0;
            float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc;
            float ddn = dot(r.d, nl);
            float cos2t = 1 - nnt * nnt * (1 - ddn * ddn);
            if (cos2t < 0) {
                r = refl_ray;
                continue;
            }

            float3 tdir = normalize(r.d * nnt - n * (into ? 1 : -1) * ddn * nnt + sqrt(cos2t));
            float a = nt - nc, b = nt + nc, R0 = a * a / (b * b);
            float c = 1 - (into ? -ddn : dot(tdir, n));
            float Re = R0 + (1 - R0) * c * c * c * c * c;
            float Tr = 1 - Re;
            float P = 0.25 + 0.25 * Re;
            float RP = Re / P;
            float TP = Tr / (1 - P);
            r = refl_ray;
            c *= Re;
        }
    }

    return col;
}

float3 ray(float2 uv, float aspect, float3 fwd, float3 up) {
    float3 right = normalize(cross(fwd, up));
    up = normalize(cross(right, fwd));
    return normalize(fwd + right * (uv.x - 0.5f) + up * -(uv.y - 0.5f) * aspect);
}

kernel void render(write_only image2d_t target, uint timestamp) {
    int2 pix = (int2){get_global_id(0), get_global_id(1)};
    float2 dim = convert_float2(get_image_dim(target));
    float2 uv = (convert_float2(pix) + 0.5f) / dim;
    float3 fwd = {0, -0.042612f, -1};
    float3 up = {0, 1, 0};
    float3 rd = ray(uv, dim.y / dim.x, fwd, up);
    struct ray r = {{50, 52, 295.6}, rd};
    float seed = uv.x * 10 + uv.y;

    float3 c = {};
    for (int i = 0; i < 1024; ++i) {
        c += radiance(r, 5, &seed);
    }
    c /= 1024.f;

    write_imagef(target, pix, (float4){c.x, c.y, c.z, 1});
}
)";

int main() {
    log::LOGGER.addSink<log::ConsoleSink>();

    std::vector<uint32_t> timestamps;
    for (size_t i = 0; i < 1; ++i) {
        timestamps.push_back(i);
    }

    try {
        auto renderer = echidna::client::Renderer(8);
        auto task = renderer.createRenderTask({
            kernel,
            timestamps,
            0,
            60,
            800,
            600
        });

        renderer.runUntilCompletion(task);
    } catch (const echidna::client::NoDeviceException& err) {
        log::write("Failed to initialize any OpenCL device");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
