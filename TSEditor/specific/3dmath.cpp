#include "../tomb5/pch.h"
#include "3dmath.h"
#include "d3dmatrix.h"
#include "dxshell.h"
#include "winmain.h"
#include "../game/spotcam.h"
#include "../game/control.h"
#include "../game/lara.h"

PHD_VECTOR CamPos;
SVECTOR CamRot;

float one = 2048.0F * float(1 << W2V_SHIFT);
float mone = 2048.0F;

float f_centerx;
float f_centery;
float f_top;
float f_left;
float f_bottom;
float f_right;
float f_znear;
float f_zfar;
float f_mznear;
float f_mzfar;
float f_persp;
float f_mpersp;
float f_oneopersp;
float f_moneopersp;
float f_perspoznear;
float f_mperspoznear;
float f_moneoznear;
float f_a;
float f_b;
float f_boo;

float fcossin_tbl[65536];

static D3DMATRIX D3DMW2VMatrix;

float* aMXPtr;
static float aW2VMx[indices_count];
float aFMatrixStack[20 * indices_count];

long* phd_mxptr;
long w2v_matrix[indices_count];
long matrix_stack[20 * indices_count];

long phd_winheight;
long phd_winwidth;
long phd_centerx;
long phd_centery;
long phd_top;
long phd_left;
long phd_bottom;
long phd_right;
long phd_znear;
long phd_zfar;
long phd_persp;
short phd_winxmax;
short phd_winxmin;
short phd_winymax;
short phd_winymin;

void AlterFOV(short fov)
{
	long fov_width;
	static short old_fov = 0;

	if (fov)
		old_fov = fov;
	else
		fov = old_fov;

	CurrentFov = fov;
	fov /= 2;
	fov_width = phd_winheight * 320 / 240;
	phd_persp = (fov_width / 2) * phd_cos(fov) / phd_sin(fov);
	f_persp = float(phd_persp);
	f_oneopersp = one / f_persp;
	f_perspoznear = f_persp / f_znear;
	f_mpersp = f_persp;
	f_moneopersp = mone / f_persp;
	f_mperspoznear = f_persp / f_mznear;
}

static void aInitMatrix()
{
	float ang;

	for (int i = 0; i < 65536; i++)
	{
		ang = (float)i * float(M_PI * 2.0F / 65536.0F);
		fcossin_tbl[i] = sin(ang);
	}

	aMXPtr = aFMatrixStack;
}

void aSetViewMatrix()
{
	SetD3DMatrix(&D3DMView, aMXPtr);
	DXAttempt(App.dx.lpD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &D3DMView));
}

static void aPushMatrix()
{
	aMXPtr[indices_count + M00] = aMXPtr[M00];
	aMXPtr[indices_count + M01] = aMXPtr[M01];
	aMXPtr[indices_count + M02] = aMXPtr[M02];
	aMXPtr[indices_count + M03] = aMXPtr[M03];
	aMXPtr[indices_count + M10] = aMXPtr[M10];
	aMXPtr[indices_count + M11] = aMXPtr[M11];
	aMXPtr[indices_count + M12] = aMXPtr[M12];
	aMXPtr[indices_count + M13] = aMXPtr[M13];
	aMXPtr[indices_count + M20] = aMXPtr[M20];
	aMXPtr[indices_count + M21] = aMXPtr[M21];
	aMXPtr[indices_count + M22] = aMXPtr[M22];
	aMXPtr[indices_count + M23] = aMXPtr[M23];
	aMXPtr += indices_count;
}

void phd_PushMatrix()
{
	phd_mxptr[indices_count + M00] = phd_mxptr[M00];
	phd_mxptr[indices_count + M01] = phd_mxptr[M01];
	phd_mxptr[indices_count + M02] = phd_mxptr[M02];
	phd_mxptr[indices_count + M03] = phd_mxptr[M03];
	phd_mxptr[indices_count + M10] = phd_mxptr[M10];
	phd_mxptr[indices_count + M11] = phd_mxptr[M11];
	phd_mxptr[indices_count + M12] = phd_mxptr[M12];
	phd_mxptr[indices_count + M13] = phd_mxptr[M13];
	phd_mxptr[indices_count + M20] = phd_mxptr[M20];
	phd_mxptr[indices_count + M21] = phd_mxptr[M21];
	phd_mxptr[indices_count + M22] = phd_mxptr[M22];
	phd_mxptr[indices_count + M23] = phd_mxptr[M23];
	phd_mxptr += indices_count;

	aPushMatrix();
}

static void aPushUnitMatrix()
{
	aMXPtr += indices_count;
	aMXPtr[M00] = 1.0F;
	aMXPtr[M01] = 0;
	aMXPtr[M02] = 0;
	aMXPtr[M03] = 0;
	aMXPtr[M10] = 0;
	aMXPtr[M11] = 1.0F;
	aMXPtr[M12] = 0;
	aMXPtr[M13] = 0;
	aMXPtr[M20] = 0;
	aMXPtr[M21] = 0;
	aMXPtr[M22] = 1.0F;
	aMXPtr[M23] = 0;
}

void phd_PushUnitMatrix()
{
	phd_mxptr += indices_count;
	phd_mxptr[M00] = 1 << W2V_SHIFT;
	phd_mxptr[M01] = 0;
	phd_mxptr[M02] = 0;
	phd_mxptr[M03] = 0;
	phd_mxptr[M10] = 0;
	phd_mxptr[M11] = 1 << W2V_SHIFT;
	phd_mxptr[M12] = 0;
	phd_mxptr[M13] = 0;
	phd_mxptr[M20] = 0;
	phd_mxptr[M21] = 0;
	phd_mxptr[M22] = 1 << W2V_SHIFT;
	phd_mxptr[M23] = 0;

	aPushUnitMatrix();
}

static void aPopMatrix()
{
	aMXPtr -= indices_count;
}

void phd_PopMatrix()
{
	phd_mxptr -= indices_count;
	aPopMatrix();
}

static void aSetTrans(long x, long y, long z)
{
	aMXPtr[M03] = (float)x;
	aMXPtr[M13] = (float)y;
	aMXPtr[M23] = (float)z;
}

void phd_SetTrans(long x, long y, long z)
{
	phd_mxptr[M03] = x << W2V_SHIFT;
	phd_mxptr[M13] = y << W2V_SHIFT;
	phd_mxptr[M23] = z << W2V_SHIFT;

	aSetTrans(x, y, z);
}

static void aTranslateRel(long x, long y, long z)
{
	aMXPtr[M03] += x * aMXPtr[M00] + y * aMXPtr[M01] + z * aMXPtr[M02];
	aMXPtr[M13] += x * aMXPtr[M10] + y * aMXPtr[M11] + z * aMXPtr[M12];
	aMXPtr[M23] += x * aMXPtr[M20] + y * aMXPtr[M21] + z * aMXPtr[M22];
}

void phd_TranslateRel(long x, long y, long z)
{
	phd_mxptr[M03] += x * phd_mxptr[M00] + y * phd_mxptr[M01] + z * phd_mxptr[M02];
	phd_mxptr[M13] += x * phd_mxptr[M10] + y * phd_mxptr[M11] + z * phd_mxptr[M12];
	phd_mxptr[M23] += x * phd_mxptr[M20] + y * phd_mxptr[M21] + z * phd_mxptr[M22];

	aTranslateRel(x, y, z);
}

static void aTranslateAbs(long x, long y, long z)
{
	float fx, fy, fz;

	fx = x - aW2VMx[M03];
	fy = y - aW2VMx[M13];
	fz = z - aW2VMx[M23];
	aMXPtr[M03] = fx * aMXPtr[M00] + fy * aMXPtr[M01] + fz * aMXPtr[M02];
	aMXPtr[M13] = fx * aMXPtr[M10] + fy * aMXPtr[M11] + fz * aMXPtr[M12];
	aMXPtr[M23] = fx * aMXPtr[M20] + fy * aMXPtr[M21] + fz * aMXPtr[M22];
}

void phd_TranslateAbs(long x, long y, long z)
{
	long fx, fy, fz;

	fx = x - w2v_matrix[M03];
	fy = y - w2v_matrix[M13];
	fz = z - w2v_matrix[M23];
	phd_mxptr[M03] = fx * phd_mxptr[M00] + fy * phd_mxptr[M01] + fz * phd_mxptr[M02];
	phd_mxptr[M13] = fx * phd_mxptr[M10] + fy * phd_mxptr[M11] + fz * phd_mxptr[M12];
	phd_mxptr[M23] = fx * phd_mxptr[M20] + fy * phd_mxptr[M21] + fz * phd_mxptr[M22];

	aTranslateAbs(x, y, z);
}

static void aRotX(short angle)
{
	float sin, cos, mx1, mx2;

	if (angle)
	{
		sin = fSin(angle);
		cos = fCos(angle);

		mx1 = cos * aMXPtr[M01] + sin * aMXPtr[M02];
		mx2 = cos * aMXPtr[M02] - sin * aMXPtr[M01];
		aMXPtr[M01] = mx1;
		aMXPtr[M02] = mx2;

		mx1 = cos * aMXPtr[M11] + sin * aMXPtr[M12];
		mx2 = cos * aMXPtr[M12] - sin * aMXPtr[M11];
		aMXPtr[M11] = mx1;
		aMXPtr[M12] = mx2;

		mx1 = cos * aMXPtr[M21] + sin * aMXPtr[M22];
		mx2 = cos * aMXPtr[M22] - sin * aMXPtr[M21];
		aMXPtr[M21] = mx1;
		aMXPtr[M22] = mx2;
	}
}

void phd_RotX(short angle)
{
	long sin, cos, mx1, mx2;

	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);

		mx1 = cos * phd_mxptr[M01] + sin * phd_mxptr[M02];
		mx2 = cos * phd_mxptr[M02] - sin * phd_mxptr[M01];
		phd_mxptr[M01] = mx1 >> W2V_SHIFT;
		phd_mxptr[M02] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M11] + sin * phd_mxptr[M12];
		mx2 = cos * phd_mxptr[M12] - sin * phd_mxptr[M11];
		phd_mxptr[M11] = mx1 >> W2V_SHIFT;
		phd_mxptr[M12] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M21] + sin * phd_mxptr[M22];
		mx2 = cos * phd_mxptr[M22] - sin * phd_mxptr[M21];
		phd_mxptr[M21] = mx1 >> W2V_SHIFT;
		phd_mxptr[M22] = mx2 >> W2V_SHIFT;
	}

	aRotX(angle);
}

static void aRotY(short angle)
{
	float sin, cos, mx1, mx2;

	if (angle)
	{
		sin = fSin(angle);
		cos = fCos(angle);

		mx1 = cos * aMXPtr[M00] - sin * aMXPtr[M02];
		mx2 = cos * aMXPtr[M02] + sin * aMXPtr[M00];
		aMXPtr[M00] = mx1;
		aMXPtr[M02] = mx2;

		mx1 = cos * aMXPtr[M10] - sin * aMXPtr[M12];
		mx2 = cos * aMXPtr[M12] + sin * aMXPtr[M10];
		aMXPtr[M10] = mx1;
		aMXPtr[M12] = mx2;

		mx1 = cos * aMXPtr[M20] - sin * aMXPtr[M22];
		mx2 = cos * aMXPtr[M22] + sin * aMXPtr[M20];
		aMXPtr[M20] = mx1;
		aMXPtr[M22] = mx2;
	}
}

void phd_RotY(short angle)
{
	long sin, cos, mx1, mx2;

	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);

		mx1 = cos * phd_mxptr[M00] - sin * phd_mxptr[M02];
		mx2 = cos * phd_mxptr[M02] + sin * phd_mxptr[M00];
		phd_mxptr[M00] = mx1 >> W2V_SHIFT;
		phd_mxptr[M02] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M10] - sin * phd_mxptr[M12];
		mx2 = cos * phd_mxptr[M12] + sin * phd_mxptr[M10];
		phd_mxptr[M10] = mx1 >> W2V_SHIFT;
		phd_mxptr[M12] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M20] - sin * phd_mxptr[M22];
		mx2 = cos * phd_mxptr[M22] + sin * phd_mxptr[M20];
		phd_mxptr[M20] = mx1 >> W2V_SHIFT;
		phd_mxptr[M22] = mx2 >> W2V_SHIFT;
	}

	aRotY(angle);
}

static void aRotZ(short angle)
{
	float sin, cos, mx1, mx2;

	if (angle)
	{
		sin = fSin(angle);
		cos = fCos(angle);

		mx1 = cos * aMXPtr[M00] + sin * aMXPtr[M01];
		mx2 = cos * aMXPtr[M01] - sin * aMXPtr[M00];
		aMXPtr[M00] = mx1;
		aMXPtr[M01] = mx2;

		mx1 = cos * aMXPtr[M10] + sin * aMXPtr[M11];
		mx2 = cos * aMXPtr[M11] - sin * aMXPtr[M10];
		aMXPtr[M10] = mx1;
		aMXPtr[M11] = mx2;

		mx1 = cos * aMXPtr[M20] + sin * aMXPtr[M21];
		mx2 = cos * aMXPtr[M21] - sin * aMXPtr[M20];;
		aMXPtr[M20] = mx1;
		aMXPtr[M21] = mx2;
	}
}

void phd_RotZ(short angle)
{
	long sin, cos, mx1, mx2;

	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);

		mx1 = cos * phd_mxptr[M00] + sin * phd_mxptr[M01];
		mx2 = cos * phd_mxptr[M01] - sin * phd_mxptr[M00];
		phd_mxptr[M00] = mx1 >> W2V_SHIFT;
		phd_mxptr[M01] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M10] + sin * phd_mxptr[M11];
		mx2 = cos * phd_mxptr[M11] - sin * phd_mxptr[M10];
		phd_mxptr[M10] = mx1 >> W2V_SHIFT;
		phd_mxptr[M11] = mx2 >> W2V_SHIFT;

		mx1 = cos * phd_mxptr[M20] + sin * phd_mxptr[M21];
		mx2 = cos * phd_mxptr[M21] - sin * phd_mxptr[M20];
		phd_mxptr[M20] = mx1 >> W2V_SHIFT;
		phd_mxptr[M21] = mx2 >> W2V_SHIFT;
	}

	aRotZ(angle);
}

void phd_RotYXZ(short y, short x, short z)
{
	if (y)
		phd_RotY(y);

	if (x)
		phd_RotX(x);

	if (z)
		phd_RotZ(z);
}

void phd_RotYXZpack(long angles)	//angles is XYZ
{
	short angle;

	angle = (angles >> 10) & 0x3FF;	//second ten bits, Y
	angle <<= 6;

	if (angle)
		phd_RotY(angle);

	angle = (angles >> 20) & 0x3FF;	//first ten bits, X
	angle <<= 6;

	if (angle)
		phd_RotX(angle);

	angle = angles & 0x3FF;			//last ten, Z
	angle <<= 6;

	if (angle)
		phd_RotZ(angle);
}

static void aScaleCurrentMatrix(PHD_VECTOR* vec)
{
	float x, y, z;

	x = vec->x * (1.0F / float(1 << W2V_SHIFT));
	y = vec->y * (1.0F / float(1 << W2V_SHIFT));
	z = vec->z * (1.0F / float(1 << W2V_SHIFT));

	aMXPtr[M00] = aMXPtr[M00] * x;
	aMXPtr[M10] = aMXPtr[M10] * x;
	aMXPtr[M20] = aMXPtr[M20] * x;

	aMXPtr[M01] = aMXPtr[M01] * y;
	aMXPtr[M11] = aMXPtr[M11] * y;
	aMXPtr[M21] = aMXPtr[M21] * y;

	aMXPtr[M02] = aMXPtr[M02] * z;
	aMXPtr[M12] = aMXPtr[M12] * z;
	aMXPtr[M22] = aMXPtr[M22] * z;
}

void ScaleCurrentMatrix(PHD_VECTOR* vec)
{
	phd_mxptr[M00] = (phd_mxptr[M00] * vec->x) >> W2V_SHIFT;
	phd_mxptr[M10] = (phd_mxptr[M10] * vec->x) >> W2V_SHIFT;
	phd_mxptr[M20] = (phd_mxptr[M20] * vec->x) >> W2V_SHIFT;

	phd_mxptr[M01] = (phd_mxptr[M01] * vec->y) >> W2V_SHIFT;
	phd_mxptr[M11] = (phd_mxptr[M11] * vec->y) >> W2V_SHIFT;
	phd_mxptr[M21] = (phd_mxptr[M21] * vec->y) >> W2V_SHIFT;

	phd_mxptr[M02] = (phd_mxptr[M02] * vec->z) >> W2V_SHIFT;
	phd_mxptr[M12] = (phd_mxptr[M12] * vec->z) >> W2V_SHIFT;
	phd_mxptr[M22] = (phd_mxptr[M22] * vec->z) >> W2V_SHIFT;

	aScaleCurrentMatrix(vec);
}

void phd_GetVectorAngles(long x, long y, long z, short* angles)
{
	short atan;

	angles[0] = (short)phd_atan(z, x);

	while ((short)x != x || (short)y != y || (short)z != z)
	{
		x >>= 2;
		y >>= 2;
		z >>= 2;
	}

	atan = (short)phd_atan(phd_sqrt(SQUARE(z) + SQUARE(x)), y);

	if ((y > 0 && atan > 0) || (y < 0 && atan < 0))
		atan = -atan;

	angles[1] = atan;
}

static void aGenerateW2V(PHD_3DPOS* viewPos)
{
	PHD_VECTOR scalar;
	float sx, cx, sy, cy, sz, cz;

	sx = fSin(viewPos->x_rot);
	cx = fCos(viewPos->x_rot);
	sy = fSin(viewPos->y_rot);
	cy = fCos(viewPos->y_rot);
	sz = fSin(viewPos->z_rot);
	cz = fCos(viewPos->z_rot);
	aMXPtr = aFMatrixStack;

	aW2VMx[M00] = FTRIGMULT3(sx, sy, sz) + FTRIGMULT2(cy, cz);
	aW2VMx[M01] = FTRIGMULT2(cx, sz);
	aW2VMx[M02] = FTRIGMULT3(sx, cy, sz) - FTRIGMULT2(sy, cz);
	aMXPtr[M00] = aW2VMx[M00];
	aMXPtr[M01] = aW2VMx[M01];
	aMXPtr[M02] = aW2VMx[M02];

	aW2VMx[M10] = FTRIGMULT3(sx, sy, cz) - FTRIGMULT2(cy, sz);
	aW2VMx[M11] = FTRIGMULT2(cx, cz);
	aW2VMx[M12] = FTRIGMULT3(sx, cy, cz) + FTRIGMULT2(sy, sz);
	aMXPtr[M10] = aW2VMx[M10];
	aMXPtr[M11] = aW2VMx[M11];
	aMXPtr[M12] = aW2VMx[M12];

	aW2VMx[M20] = FTRIGMULT2(cx, sy);
	aW2VMx[M21] = -sx;
	aW2VMx[M22] = FTRIGMULT2(cx, cy);
	aMXPtr[M20] = aW2VMx[M20];
	aMXPtr[M21] = aW2VMx[M21];
	aMXPtr[M22] = aW2VMx[M22];

	if (lara.dpoisoned != lara.poisoned)
	{
		lara.poisoned += (lara.dpoisoned - lara.poisoned) >> 4;

		if (abs(lara.dpoisoned - lara.poisoned) < 16)
			lara.poisoned = lara.dpoisoned;
	}

	if (lara.poisoned >= 256)
	{
		scalar.x = (lara.poisoned - 256) * ((phd_sin(XSoff1) + phd_sin(XSoff2)) >> 2);
		scalar.y = (lara.poisoned - 256) * ((phd_sin(YSoff1) + phd_sin(YSoff2)) >> 2);
		scalar.z = (lara.poisoned - 256) * ((phd_sin(ZSoff1) + phd_sin(ZSoff2)) >> 2);

		if (scalar.x || scalar.y || scalar.z)
		{
			scalar.x = (scalar.x >> 12) + 0x4000;
			scalar.y = (scalar.y >> 12) + 0x4000;
			scalar.z = (scalar.z >> 12) + 0x4000;
			ScaleCurrentMatrix(&scalar);
		}
	}

	aW2VMx[M03] = (float)viewPos->x_pos;
	aW2VMx[M13] = (float)viewPos->y_pos;
	aW2VMx[M23] = (float)viewPos->z_pos;
	aMXPtr[M03] = aW2VMx[M03];
	aMXPtr[M13] = aW2VMx[M13];
	aMXPtr[M23] = aW2VMx[M23];

	aW2VMx[M10] = aMXPtr[M10];
	aW2VMx[M11] = aMXPtr[M11];
	aW2VMx[M12] = aMXPtr[M12];

	SetD3DMatrix(&D3DMW2VMatrix, aW2VMx);
	D3DInvCameraMatrix._11 = D3DMW2VMatrix._11;
	D3DInvCameraMatrix._12 = D3DMW2VMatrix._21;
	D3DInvCameraMatrix._13 = D3DMW2VMatrix._31;
	D3DInvCameraMatrix._14 = D3DMW2VMatrix._41;
	D3DInvCameraMatrix._21 = D3DMW2VMatrix._12;
	D3DInvCameraMatrix._22 = D3DMW2VMatrix._22;
	D3DInvCameraMatrix._23 = D3DMW2VMatrix._32;
	D3DInvCameraMatrix._24 = D3DMW2VMatrix._42;
	D3DInvCameraMatrix._31 = D3DMW2VMatrix._13;
	D3DInvCameraMatrix._32 = D3DMW2VMatrix._23;
	D3DInvCameraMatrix._33 = D3DMW2VMatrix._33;
	D3DInvCameraMatrix._34 = D3DMW2VMatrix._43;
	D3DInvCameraMatrix._41 = D3DMW2VMatrix._14;
	D3DInvCameraMatrix._42 = D3DMW2VMatrix._24;
	D3DInvCameraMatrix._43 = D3DMW2VMatrix._34;
	D3DInvCameraMatrix._44 = D3DMW2VMatrix._44;
}

void phd_GenerateW2V(PHD_3DPOS* viewPos)
{
	PHD_VECTOR scalar;
	long sx, cx, sy, cy, sz, cz;

	sx = phd_sin(viewPos->x_rot);
	cx = phd_cos(viewPos->x_rot);
	sy = phd_sin(viewPos->y_rot);
	cy = phd_cos(viewPos->y_rot);
	sz = phd_sin(viewPos->z_rot);
	cz = phd_cos(viewPos->z_rot);
	phd_mxptr = matrix_stack;

	w2v_matrix[M00] = TRIGMULT3(sx, sy, sz) + TRIGMULT2(cy, cz);
	w2v_matrix[M01] = TRIGMULT2(cx, sz);
	w2v_matrix[M02] = TRIGMULT3(sx, cy, sz) - TRIGMULT2(sy, cz);
	phd_mxptr[M00] = w2v_matrix[M00];
	phd_mxptr[M01] = w2v_matrix[M01];
	phd_mxptr[M02] = w2v_matrix[M02];

	w2v_matrix[M10] = TRIGMULT3(sx, sy, cz) - TRIGMULT2(cy, sz);
	w2v_matrix[M11] = TRIGMULT2(cx, cz);
	w2v_matrix[M12] = TRIGMULT3(sx, cy, cz) + TRIGMULT2(sy, sz);
	phd_mxptr[M10] = w2v_matrix[M10];
	phd_mxptr[M11] = w2v_matrix[M11];
	phd_mxptr[M12] = w2v_matrix[M12];

	w2v_matrix[M20] = TRIGMULT2(cx, sy);
	w2v_matrix[M21] = -sx;
	w2v_matrix[M22] = TRIGMULT2(cx, cy);
	phd_mxptr[M20] = w2v_matrix[M20];
	phd_mxptr[M21] = w2v_matrix[M21];
	phd_mxptr[M22] = w2v_matrix[M22];

	if (lara.dpoisoned != lara.poisoned)
	{
		lara.poisoned += (lara.dpoisoned - lara.poisoned) >> 4;

		if (abs(lara.dpoisoned - lara.poisoned) < 16)
			lara.poisoned = lara.dpoisoned;
	}

	if (lara.poisoned >= 256)
	{
		scalar.x = (lara.poisoned - 256) * ((phd_sin(XSoff1) + phd_sin(XSoff2)) >> 2);
		scalar.y = (lara.poisoned - 256) * ((phd_sin(YSoff1) + phd_sin(YSoff2)) >> 2);
		scalar.z = (lara.poisoned - 256) * ((phd_sin(ZSoff1) + phd_sin(ZSoff2)) >> 2);

		if (scalar.x || scalar.y || scalar.z)
		{
			scalar.x = (scalar.x >> 12) + 0x4000;
			scalar.y = (scalar.y >> 12) + 0x4000;
			scalar.z = (scalar.z >> 12) + 0x4000;
			ScaleCurrentMatrix(&scalar);
		}
	}

	w2v_matrix[M03] = viewPos->x_pos;;
	w2v_matrix[M13] = viewPos->y_pos;
	w2v_matrix[M23] = viewPos->z_pos;
	phd_mxptr[M03] = w2v_matrix[M03];
	phd_mxptr[M13] = w2v_matrix[M13];
	phd_mxptr[M23] = w2v_matrix[M23];

	w2v_matrix[M10] = phd_mxptr[M10];
	w2v_matrix[M11] = phd_mxptr[M11];
	w2v_matrix[M12] = phd_mxptr[M12];

	aGenerateW2V(viewPos);
}

void phd_LookAt(long xsrc, long ysrc, long zsrc, long xtar, long ytar, long ztar, short roll)
{
	PHD_3DPOS viewPos;
	long dx, dy, dz;
	short angles[2];

	phd_GetVectorAngles(xtar - xsrc, ytar - ysrc, ztar - zsrc, angles);
	viewPos.x_pos = xsrc;
	viewPos.y_pos = ysrc;
	viewPos.z_pos = zsrc;
	viewPos.x_rot = angles[1];
	viewPos.y_rot = angles[0];
	viewPos.z_rot = roll;
	dx = xsrc - xtar;
	dy = ysrc - ytar;
	dz = zsrc - ztar;
	CamRot.x = (mGetAngle(0, 0, (long)phd_sqrt(SQUARE(dx) + SQUARE(dz)), dy) >> 4) & 0xFFF;
	CamRot.y = (mGetAngle(zsrc, xsrc, ztar, xtar) >> 4) & 0xFFF;
	CamRot.z = 0;
	CamPos.x = xsrc;
	CamPos.y = ysrc;
	CamPos.z = zsrc;
	phd_GenerateW2V(&viewPos);
	S_InitD3DMatrix();
	aSetViewMatrix();
}

void SetupZRange(long znear, long zfar)
{
	phd_znear = znear;
	phd_zfar = zfar;
	f_zfar = (float)zfar;
	f_znear = (float)znear;
	f_perspoznear = f_persp / f_znear;
	f_mznear = float(znear >> W2V_SHIFT);
	f_mzfar = float(zfar >> W2V_SHIFT);
	f_mperspoznear = f_mpersp / f_mznear;
	f_moneoznear = mone / f_mznear;
	f_b = f_mzfar * f_mznear * 0.99F / (f_mznear - f_mzfar);
	f_a = 0.005F - f_b / f_mznear;
	f_b = -f_b;
	f_boo = f_b / mone;
}

void InitWindow(long x, long y, long w, long h, long znear, long zfar, long fov, long a, long b)
{
	phd_winwidth = w;
	phd_winxmax = short(w - 1);
	phd_winxmin = (short)x;
	phd_winheight = h;
	phd_winymax = short(h - 1);
	phd_winymin = (short)y;
	phd_centerx = w / 2;
	phd_centery = h / 2;
	phd_znear = znear << W2V_SHIFT;
	f_centerx = float(w / 2);
	phd_zfar = zfar << W2V_SHIFT;
	f_centery = float(h / 2);
	AlterFOV(short(182 * fov));
	SetupZRange(phd_znear, phd_zfar);
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;
	phd_left = x;
	phd_top = y;
	f_right = float(phd_winxmax + 1);
	f_bottom = float(phd_winymax + 1);
	f_top = (float)phd_winymin;
	f_left = (float)phd_winxmin;
	phd_mxptr = matrix_stack;
	aInitMatrix();
}

long phd_atan(long x, long y)
{
	long octant, n, result;

	result = 0;
	octant = 0;

	if (x || y)
	{
		if (x < 0)
		{
			octant += 4;
			x = -x;
		}

		if (y < 0)
		{
			octant += 2;
			y = -y;
		}

		if (y > x)
		{
			octant++;
			n = x;
			x = y;
			y = n;
		}

		while ((short)y != y)
		{
			x >>= 1;
			y >>= 1;
		}

		result = phdtan2[octant] + phdtantab[(y << 11) / x];

		if (result < 0)
			result = -result;
	}

	return result;
}

ulong phd_sqrt(ulong num)
{
	ulong base, result, tmp;

	base = 0x40000000;
	result = 0;

	do
	{
		tmp = result;
		result += base;
		tmp >>= 1;

		if (result > num)
			result = tmp;
		else
		{
			num -= result;
			result = base | tmp;
		}

		base >>= 2;

	} while (base);

	return result;
}

ulong mGetAngle(long x, long z, long x1, long z1)
{
	long dx, dz, octant, swap, angle;

	dx = x1 - x;
	dz = z1 - z;

	if (!dx && !dz)
		return 0;

	octant = 0;

	if (dx < 0)
	{
		octant = 4;
		dx = -dx;
	}

	if (dz < 0)
	{
		octant += 2;
		dz = -dz;
	}

	if (dz > dx)
	{
		octant++;
		swap = dx;
		dx = dz;
		dz = swap;
	}

	while ((short)dz != dz)
	{
		dx >>= 1;
		dz >>= 1;
	}

	angle = phdtan2[octant] + phdtantab[(dz << 11) / dx];

	if (angle < 0)
		angle = -angle;

	return -angle & 0xFFFF;
}
