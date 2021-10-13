#include "../utils.h"

namespace math {
	void sin_cos(float rad, float& sin, float& cos) {
		sin = math::sin(rad);
		cos = math::cos(rad);
	}

	void angle_vectors(const qangle_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up) {
		vec3_t cos, sin;

		sin_cos(deg_to_rad(angles.x), sin.x, cos.x);
		sin_cos(deg_to_rad(angles.y), sin.y, cos.y);
		sin_cos(deg_to_rad(angles.z), sin.z, cos.z);

		if (forward) {
			forward->x = cos.x * cos.y;
			forward->y = cos.x * sin.y;
			forward->z = -sin.x;
		}

		if (right) {
			right->x = -sin.z * sin.x * cos.y + -cos.z * -sin.y;
			right->y = -sin.z * sin.x * sin.y + -cos.z * cos.y;
			right->z = -sin.z * cos.x;
		}

		if (up) {
			up->x = cos.z * sin.x * cos.y + -sin.z * -sin.y;
			up->y = cos.z * sin.x * sin.y + -sin.z * cos.y;
			up->z = cos.z * cos.x;
		}
	}

	qangle_t calc_angle(const vec3_t& src, const vec3_t& dst) {
		qangle_t vAngle;
		vec3_t delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
		double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);

		vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
		vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
		vAngle.z = 0.0f;

		if (delta.x >= 0.0)
			vAngle.y += 180.0f;

		return vAngle;
	}

    float normalize_yaw(float yaw) {
        while (yaw < -180.f)
            yaw += 360.f;
        while (yaw > 180.f)
            yaw -= 360.f;
        return yaw;
    }

    void matrix_copy(const matrix3x4_t& in, matrix3x4_t& out) {
        memcpy(&out, &in, sizeof(float) * 3 * 4);
    }

    void angle_matrix(const qangle_t& angles, matrix3x4_t& matrix) {
#ifdef _VPROF_MATHLIB
        VPROF_BUDGET("angle_matrix", "Mathlib");
#endif

        float sr, sp, sy, cr, cp, cy;

#ifdef _X360
        fltx4 radians, scale, sine, cosine;
        radians = LoadUnaligned3SIMD(angles.Base());
        scale = ReplicateX4(M_PI_F / 180.f);
        radians = MulSIMD(radians, scale);
        SinCos3SIMD(sine, cosine, radians);

        sp = SubFloat(sine, 0);	sy = SubFloat(sine, 1);	sr = SubFloat(sine, 2);
        cp = SubFloat(cosine, 0);	cy = SubFloat(cosine, 1);	cr = SubFloat(cosine, 2);
#else
        math::sin_cos(math::deg_to_rad(angles.x), sy, cy);
        math::sin_cos(math::deg_to_rad(angles.y), sp, cp);
        math::sin_cos(math::deg_to_rad(angles.z), sr, cr);
#endif

        matrix[0][0] = cp * cy;
        matrix[1][0] = cp * sy;
        matrix[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;
        matrix[0][1] = sp * srcy - crsy;
        matrix[1][1] = sp * srsy + crcy;
        matrix[2][1] = sr * cp;

        matrix[0][2] = (sp * crcy + srsy);
        matrix[1][2] = (sp * crsy - srcy);
        matrix[2][2] = cr * cp;

        matrix[0][3] = 0.0f;
        matrix[1][3] = 0.0f;
        matrix[2][3] = 0.0f;
    }

    void concat_transform(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out) {
        if (&in1 == &out) {
            matrix3x4_t in1b;
            matrix_copy(in1, in1b);
            concat_transform(in1b, in2, out);
            return;
        }
        if (&in2 == &out) {
            matrix3x4_t in2b;
            matrix_copy(in2, in2b);
            concat_transform(in1, in2b, out);
            return;
        }

        out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
            in1[0][2] * in2[2][0];
        out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
            in1[0][2] * in2[2][1];
        out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
            in1[0][2] * in2[2][2];
        out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
            in1[0][2] * in2[2][3] + in1[0][3];
        out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
            in1[1][2] * in2[2][0];
        out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
            in1[1][2] * in2[2][1];
        out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
            in1[1][2] * in2[2][2];
        out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
            in1[1][2] * in2[2][3] + in1[1][3];
        out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
            in1[2][2] * in2[2][0];
        out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
            in1[2][2] * in2[2][1];
        out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
            in1[2][2] * in2[2][2];
        out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
            in1[2][2] * in2[2][3] + in1[2][3];
    }

    void vector_transform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out) {
        out[0] = in1.dot_product(in2[0]) + in2[0][3];
        out[1] = in1.dot_product(in2[1]) + in2[1][3];
        out[2] = in1.dot_product(in2[2]) + in2[2][3];
    }

    vec3_t cross_product(const vec3_t& a, const vec3_t& b) {
        return vec3_t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    }

    void vector_angles(const vec3_t& forward, vec3_t& up, qangle_t& angles) {

        vec3_t left = cross_product(up, forward);
        left.normalize();

        float forwardDist = forward.length_2d();

        if (forwardDist > 0.001f) {
            angles.x = atan2f(-forward.z, forwardDist) * 180 / math::m_pi;
            angles.y = atan2f(forward.y, forward.x) * 180 / math::m_pi;

            float upZ = (left.y * forward.x) - (left.x * forward.y);
            angles.z = atan2f(left.z, upZ) * 180 / math::m_pi;
        }
        else {
            angles.x = atan2f(-forward.z, forwardDist) * 180 / math::m_pi;
            angles.y = atan2f(-left.x, left.y) * 180 / math::m_pi;
            angles.z = 0;
        }
    }

    void vector_angles(const vec3_t& forward, qangle_t& angles) {
        float tmp, yaw, pitch;

        if (forward[1] == 0 && forward[0] == 0) {
            yaw = 0;
            if (forward[2] > 0)
                pitch = 270;
            else
                pitch = 90;
        }
        else {
            yaw = (atan2(forward[1], forward[0]) * 180 / math::m_pi);
            if (yaw < 0)
                yaw += 360;

            tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
            pitch = (atan2(-forward[2], tmp) * 180 / math::m_pi);
            if (pitch < 0)
                pitch += 360;
        }

        angles.x = pitch;
        angles.y = yaw;
        angles.z = 0;
    }

    float calc_fov(qangle_t& view_angle, const qangle_t& aim_angle) {
        vec3_t view, aim;

        angle_vectors(view_angle, &aim);
        angle_vectors(aim_angle, &view);

        return math::rad_to_deg(std::acos(aim.dot_product(view) / aim.length_sqr()));
    }

    float random_float(float min, float max) {
        static auto random = reinterpret_cast<float(*)(float, float)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "RandomFloat"));
        if (random)
            return random(min, max);
        else
            return 0.f;
    }
}
