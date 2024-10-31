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

FMatrix* aMXPtr;
static FMatrix aW2VMx;
FMatrix aFMatrixStack[20];

Matrix* phd_mxptr;
Matrix w2v_matrix;
Matrix matrix_stack[20];

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
	aMXPtr[1] = aMXPtr[0];
	aMXPtr++;
}

void phd_PushMatrix()
{
	phd_mxptr[1] = phd_mxptr[0];
	phd_mxptr++;
	aPushMatrix();
}

static void aPushUnitMatrix()
{
	aMXPtr++;
	aMXPtr->m00 = 1.0F;
	aMXPtr->m01 = 0;
	aMXPtr->m02 = 0;
	aMXPtr->m03 = 0;
	aMXPtr->m10 = 0;
	aMXPtr->m11 = 1.0F;
	aMXPtr->m12 = 0;
	aMXPtr->m13 = 0;
	aMXPtr->m20 = 0;
	aMXPtr->m21 = 0;
	aMXPtr->m22 = 1.0F;
	aMXPtr->m23 = 0;
}

void phd_PushUnitMatrix()
{
	phd_mxptr++;
	phd_mxptr->m00 = 1 << W2V_SHIFT;
	phd_mxptr->m01 = 0;
	phd_mxptr->m02 = 0;
	phd_mxptr->m03 = 0;
	phd_mxptr->m10 = 0;
	phd_mxptr->m11 = 1 << W2V_SHIFT;
	phd_mxptr->m12 = 0;
	phd_mxptr->m13 = 0;
	phd_mxptr->m20 = 0;
	phd_mxptr->m21 = 0;
	phd_mxptr->m22 = 1 << W2V_SHIFT;
	phd_mxptr->m23 = 0;
	aPushUnitMatrix();
}

static void aPopMatrix()
{
	aMXPtr--;
}

void phd_PopMatrix()
{
	phd_mxptr--;
	aPopMatrix();
}

static void aSetTrans(long x, long y, long z)
{
	aMXPtr->m03 = (float)x;
	aMXPtr->m13 = (float)y;
	aMXPtr->m23 = (float)z;
}

void phd_SetTrans(long x, long y, long z)
{
	phd_mxptr->m03 = x << W2V_SHIFT;
	phd_mxptr->m13 = y << W2V_SHIFT;
	phd_mxptr->m23 = z << W2V_SHIFT;
	aSetTrans(x, y, z);
}

static void aTranslateRel(long x, long y, long z)
{
	aMXPtr->m03 += x * aMXPtr->m00 + y * aMXPtr->m01 + z * aMXPtr->m02;
	aMXPtr->m13 += x * aMXPtr->m10 + y * aMXPtr->m11 + z * aMXPtr->m12;
	aMXPtr->m23 += x * aMXPtr->m20 + y * aMXPtr->m21 + z * aMXPtr->m22;
}

void phd_TranslateRel(long x, long y, long z)
{
	phd_mxptr->m03 += x * phd_mxptr->m00 + y * phd_mxptr->m01 + z * phd_mxptr->m02;
	phd_mxptr->m13 += x * phd_mxptr->m10 + y * phd_mxptr->m11 + z * phd_mxptr->m12;
	phd_mxptr->m23 += x * phd_mxptr->m20 + y * phd_mxptr->m21 + z * phd_mxptr->m22;
	aTranslateRel(x, y, z);
}

static void aTranslateAbs(long x, long y, long z)
{
	float fx = x - aW2VMx.m03;
	float fy = y - aW2VMx.m13;
	float fz = z - aW2VMx.m23;
	aMXPtr->m03 = fx * aMXPtr->m00 + fy * aMXPtr->m01 + fz * aMXPtr->m02;
	aMXPtr->m13 = fx * aMXPtr->m10 + fy * aMXPtr->m11 + fz * aMXPtr->m12;
	aMXPtr->m23 = fx * aMXPtr->m20 + fy * aMXPtr->m21 + fz * aMXPtr->m22;
}

void phd_TranslateAbs(long x, long y, long z)
{
	int fx = x - w2v_matrix.m03;
	int fy = y - w2v_matrix.m13;
	int fz = z - w2v_matrix.m23;
	phd_mxptr->m03 = fx * phd_mxptr->m00 + fy * phd_mxptr->m01 + fz * phd_mxptr->m02;
	phd_mxptr->m13 = fx * phd_mxptr->m10 + fy * phd_mxptr->m11 + fz * phd_mxptr->m12;
	phd_mxptr->m23 = fx * phd_mxptr->m20 + fy * phd_mxptr->m21 + fz * phd_mxptr->m22;
	aTranslateAbs(x, y, z);
}

static void aRotX(short angle)
{
	float sin, cos, mx1, mx2;
	if (angle)
	{
		sin = fSin(angle);
		cos = fCos(angle);
		mx1 = cos * aMXPtr->m01 + sin * aMXPtr->m02;
		mx2 = cos * aMXPtr->m02 - sin * aMXPtr->m01;
		aMXPtr->m01 = mx1;
		aMXPtr->m02 = mx2;
		mx1 = cos * aMXPtr->m11 + sin * aMXPtr->m12;
		mx2 = cos * aMXPtr->m12 - sin * aMXPtr->m11;
		aMXPtr->m11 = mx1;
		aMXPtr->m12 = mx2;
		mx1 = cos * aMXPtr->m21 + sin * aMXPtr->m22;
		mx2 = cos * aMXPtr->m22 - sin * aMXPtr->m21;
		aMXPtr->m21 = mx1;
		aMXPtr->m22 = mx2;
	}
}

void phd_RotX(short angle)
{
	long sin, cos, mx1, mx2;
	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);
		mx1 = cos * phd_mxptr->m01 + sin * phd_mxptr->m02;
		mx2 = cos * phd_mxptr->m02 - sin * phd_mxptr->m01;
		phd_mxptr->m01 = mx1 >> W2V_SHIFT;
		phd_mxptr->m02 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m11 + sin * phd_mxptr->m12;
		mx2 = cos * phd_mxptr->m12 - sin * phd_mxptr->m11;
		phd_mxptr->m11 = mx1 >> W2V_SHIFT;
		phd_mxptr->m12 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m21 + sin * phd_mxptr->m22;
		mx2 = cos * phd_mxptr->m22 - sin * phd_mxptr->m21;
		phd_mxptr->m21 = mx1 >> W2V_SHIFT;
		phd_mxptr->m22 = mx2 >> W2V_SHIFT;
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
		mx1 = cos * aMXPtr->m00 - sin * aMXPtr->m02;
		mx2 = cos * aMXPtr->m02 + sin * aMXPtr->m00;
		aMXPtr->m00 = mx1;
		aMXPtr->m02 = mx2;
		mx1 = cos * aMXPtr->m10 - sin * aMXPtr->m12;
		mx2 = cos * aMXPtr->m12 + sin * aMXPtr->m10;
		aMXPtr->m10 = mx1;
		aMXPtr->m12 = mx2;
		mx1 = cos * aMXPtr->m20 - sin * aMXPtr->m22;
		mx2 = cos * aMXPtr->m22 + sin * aMXPtr->m20;
		aMXPtr->m20 = mx1;
		aMXPtr->m22 = mx2;
	}
}

void phd_RotY(short angle)
{
	long sin, cos, mx1, mx2;
	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);
		mx1 = cos * phd_mxptr->m00 - sin * phd_mxptr->m02;
		mx2 = cos * phd_mxptr->m02 + sin * phd_mxptr->m00;
		phd_mxptr->m00 = mx1 >> W2V_SHIFT;
		phd_mxptr->m02 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m10 - sin * phd_mxptr->m12;
		mx2 = cos * phd_mxptr->m12 + sin * phd_mxptr->m10;
		phd_mxptr->m10 = mx1 >> W2V_SHIFT;
		phd_mxptr->m12 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m20 - sin * phd_mxptr->m22;
		mx2 = cos * phd_mxptr->m22 + sin * phd_mxptr->m20;
		phd_mxptr->m20 = mx1 >> W2V_SHIFT;
		phd_mxptr->m22 = mx2 >> W2V_SHIFT;
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
		mx1 = cos * aMXPtr->m00 + sin * aMXPtr->m01;
		mx2 = cos * aMXPtr->m01 - sin * aMXPtr->m00;
		aMXPtr->m00 = mx1;
		aMXPtr->m01 = mx2;
		mx1 = cos * aMXPtr->m10 + sin * aMXPtr->m11;
		mx2 = cos * aMXPtr->m11 - sin * aMXPtr->m10;
		aMXPtr->m10 = mx1;
		aMXPtr->m11 = mx2;
		mx1 = cos * aMXPtr->m20 + sin * aMXPtr->m21;
		mx2 = cos * aMXPtr->m21 - sin * aMXPtr->m20;;
		aMXPtr->m20 = mx1;
		aMXPtr->m21 = mx2;
	}
}

void phd_RotZ(short angle)
{
	long sin, cos, mx1, mx2;
	if (angle)
	{
		sin = phd_sin(angle);
		cos = phd_cos(angle);
		mx1 = cos * phd_mxptr->m00 + sin * phd_mxptr->m01;
		mx2 = cos * phd_mxptr->m01 - sin * phd_mxptr->m00;
		phd_mxptr->m00 = mx1 >> W2V_SHIFT;
		phd_mxptr->m01 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m10 + sin * phd_mxptr->m11;
		mx2 = cos * phd_mxptr->m11 - sin * phd_mxptr->m10;
		phd_mxptr->m10 = mx1 >> W2V_SHIFT;
		phd_mxptr->m11 = mx2 >> W2V_SHIFT;
		mx1 = cos * phd_mxptr->m20 + sin * phd_mxptr->m21;
		mx2 = cos * phd_mxptr->m21 - sin * phd_mxptr->m20;
		phd_mxptr->m20 = mx1 >> W2V_SHIFT;
		phd_mxptr->m21 = mx2 >> W2V_SHIFT;
	}
	aRotZ(angle);
}

void phd_RotYXZ(short y, short x, short z)
{
	phd_RotY(y);
	phd_RotX(x);
	phd_RotZ(z);
}

void phd_RotYXZpack(long angles)	//angles is XYZ
{
	short angle = (angles >> 10) & 0x3FF;	//second ten bits, Y
	angle <<= 6;
	if (angle) phd_RotY(angle);

	angle = (angles >> 20) & 0x3FF;	//first ten bits, X
	angle <<= 6;
	if (angle) phd_RotX(angle);

	angle = angles & 0x3FF;			//last ten, Z
	angle <<= 6;
	if (angle) phd_RotZ(angle);
}

static void aScaleCurrentMatrix(PHD_VECTOR* vec)
{
	float x = vec->x * (1.0F / float(1 << W2V_SHIFT));
	float y = vec->y * (1.0F / float(1 << W2V_SHIFT));
	float z = vec->z * (1.0F / float(1 << W2V_SHIFT));
	aMXPtr->m00 = aMXPtr->m00 * x;
	aMXPtr->m10 = aMXPtr->m10 * x;
	aMXPtr->m20 = aMXPtr->m20 * x;
	aMXPtr->m01 = aMXPtr->m01 * y;
	aMXPtr->m11 = aMXPtr->m11 * y;
	aMXPtr->m21 = aMXPtr->m21 * y;
	aMXPtr->m02 = aMXPtr->m02 * z;
	aMXPtr->m12 = aMXPtr->m12 * z;
	aMXPtr->m22 = aMXPtr->m22 * z;
}

void ScaleCurrentMatrix(PHD_VECTOR* vec)
{
	phd_mxptr->m00 = (phd_mxptr->m00 * vec->x) >> W2V_SHIFT;
	phd_mxptr->m10 = (phd_mxptr->m10 * vec->x) >> W2V_SHIFT;
	phd_mxptr->m20 = (phd_mxptr->m20 * vec->x) >> W2V_SHIFT;
	phd_mxptr->m01 = (phd_mxptr->m01 * vec->y) >> W2V_SHIFT;
	phd_mxptr->m11 = (phd_mxptr->m11 * vec->y) >> W2V_SHIFT;
	phd_mxptr->m21 = (phd_mxptr->m21 * vec->y) >> W2V_SHIFT;
	phd_mxptr->m02 = (phd_mxptr->m02 * vec->z) >> W2V_SHIFT;
	phd_mxptr->m12 = (phd_mxptr->m12 * vec->z) >> W2V_SHIFT;
	phd_mxptr->m22 = (phd_mxptr->m22 * vec->z) >> W2V_SHIFT;
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

	aW2VMx.m00 = FTRIGMULT3(sx, sy, sz) + FTRIGMULT2(cy, cz);
	aW2VMx.m01 = FTRIGMULT2(cx, sz);
	aW2VMx.m02 = FTRIGMULT3(sx, cy, sz) - FTRIGMULT2(sy, cz);
	aMXPtr->m00 = aW2VMx.m00;
	aMXPtr->m01 = aW2VMx.m01;
	aMXPtr->m02 = aW2VMx.m02;

	aW2VMx.m10 = FTRIGMULT3(sx, sy, cz) - FTRIGMULT2(cy, sz);
	aW2VMx.m11 = FTRIGMULT2(cx, cz);
	aW2VMx.m12 = FTRIGMULT3(sx, cy, cz) + FTRIGMULT2(sy, sz);
	aMXPtr->m10 = aW2VMx.m10;
	aMXPtr->m11 = aW2VMx.m11;
	aMXPtr->m12 = aW2VMx.m12;

	aW2VMx.m20 = FTRIGMULT2(cx, sy);
	aW2VMx.m21 = -sx;
	aW2VMx.m22 = FTRIGMULT2(cx, cy);
	aMXPtr->m20 = aW2VMx.m20;
	aMXPtr->m21 = aW2VMx.m21;
	aMXPtr->m22 = aW2VMx.m22;

	if (Lara.dpoisoned != Lara.poisoned)
	{
		Lara.poisoned += (Lara.dpoisoned - Lara.poisoned) >> 4;
		if (abs(Lara.dpoisoned - Lara.poisoned) < 16)
			Lara.poisoned = Lara.dpoisoned;
	}

	if (Lara.poisoned >= 256)
	{
		scalar.x = (Lara.poisoned - 256) * ((phd_sin(XSoff1) + phd_sin(XSoff2)) >> 2);
		scalar.y = (Lara.poisoned - 256) * ((phd_sin(YSoff1) + phd_sin(YSoff2)) >> 2);
		scalar.z = (Lara.poisoned - 256) * ((phd_sin(ZSoff1) + phd_sin(ZSoff2)) >> 2);
		if (scalar.x || scalar.y || scalar.z)
		{
			scalar.x = (scalar.x >> 12) + 0x4000;
			scalar.y = (scalar.y >> 12) + 0x4000;
			scalar.z = (scalar.z >> 12) + 0x4000;
			ScaleCurrentMatrix(&scalar);
		}
	}

	aW2VMx.m03 = (float)viewPos->x_pos;
	aW2VMx.m13 = (float)viewPos->y_pos;
	aW2VMx.m23 = (float)viewPos->z_pos;
	aMXPtr->m03 = aW2VMx.m03;
	aMXPtr->m13 = aW2VMx.m13;
	aMXPtr->m23 = aW2VMx.m23;
	aW2VMx.m10 = aMXPtr->m10;
	aW2VMx.m11 = aMXPtr->m11;
	aW2VMx.m12 = aMXPtr->m12;

	SetD3DMatrix(&D3DMW2VMatrix, &aW2VMx);
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

	w2v_matrix.m00 = TRIGMULT3(sx, sy, sz) + TRIGMULT2(cy, cz);
	w2v_matrix.m01 = TRIGMULT2(cx, sz);
	w2v_matrix.m02 = TRIGMULT3(sx, cy, sz) - TRIGMULT2(sy, cz);
	phd_mxptr->m00 = w2v_matrix.m00;
	phd_mxptr->m01 = w2v_matrix.m01;
	phd_mxptr->m02 = w2v_matrix.m02;

	w2v_matrix.m10 = TRIGMULT3(sx, sy, cz) - TRIGMULT2(cy, sz);
	w2v_matrix.m11 = TRIGMULT2(cx, cz);
	w2v_matrix.m12 = TRIGMULT3(sx, cy, cz) + TRIGMULT2(sy, sz);
	phd_mxptr->m10 = w2v_matrix.m10;
	phd_mxptr->m11 = w2v_matrix.m11;
	phd_mxptr->m12 = w2v_matrix.m12;

	w2v_matrix.m20 = TRIGMULT2(cx, sy);
	w2v_matrix.m21 = -sx;
	w2v_matrix.m22 = TRIGMULT2(cx, cy);
	phd_mxptr->m20 = w2v_matrix.m20;
	phd_mxptr->m21 = w2v_matrix.m21;
	phd_mxptr->m22 = w2v_matrix.m22;

	if (Lara.dpoisoned != Lara.poisoned)
	{
		Lara.poisoned += (Lara.dpoisoned - Lara.poisoned) >> 4;
		if (abs(Lara.dpoisoned - Lara.poisoned) < 16)
			Lara.poisoned = Lara.dpoisoned;
	}

	if (Lara.poisoned >= 256)
	{
		scalar.x = (Lara.poisoned - 256) * ((phd_sin(XSoff1) + phd_sin(XSoff2)) >> 2);
		scalar.y = (Lara.poisoned - 256) * ((phd_sin(YSoff1) + phd_sin(YSoff2)) >> 2);
		scalar.z = (Lara.poisoned - 256) * ((phd_sin(ZSoff1) + phd_sin(ZSoff2)) >> 2);
		if (scalar.x || scalar.y || scalar.z)
		{
			scalar.x = (scalar.x >> 12) + 0x4000;
			scalar.y = (scalar.y >> 12) + 0x4000;
			scalar.z = (scalar.z >> 12) + 0x4000;
			ScaleCurrentMatrix(&scalar);
		}
	}

	w2v_matrix.m03 = viewPos->x_pos;
	w2v_matrix.m13 = viewPos->y_pos;
	w2v_matrix.m23 = viewPos->z_pos;
	phd_mxptr->m03 = w2v_matrix.m03;
	phd_mxptr->m13 = w2v_matrix.m13;
	phd_mxptr->m23 = w2v_matrix.m23;
	w2v_matrix.m10 = phd_mxptr->m10;
	w2v_matrix.m11 = phd_mxptr->m11;
	w2v_matrix.m12 = phd_mxptr->m12;
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
	phd_zfar = zfar << W2V_SHIFT;
	phd_right = phd_winxmax;
	phd_bottom = phd_winymax;
	phd_left = x;
	phd_top = y;
	phd_mxptr = matrix_stack;

	f_right = float(phd_winxmax + 1);
	f_bottom = float(phd_winymax + 1);
	f_top = (float)phd_winymin;
	f_left = (float)phd_winxmin;
	f_centerx = float(w / 2);
	f_centery = float(h / 2);

	AlterFOV(fov);
	SetupZRange(phd_znear, phd_zfar);
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
