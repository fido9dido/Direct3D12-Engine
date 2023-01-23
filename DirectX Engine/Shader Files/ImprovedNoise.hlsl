

static float3 g[] =
{
	1,1,0,
	-1,1,0,
	1,-1,0,
	-1,-1,0,
	1,0,1,
	-1,0,1,
	1,0,-1,
	-1,0,-1,
	0,1,1,
	0,-1,1,
	0,1,-1,
	0,-1,-1,
	1,1,0,
	0,-1,1,
	-1,1,0,
	0,-1,-1,
};


groupshared  int p[512] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180

//0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,
//29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54
//,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,
//80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,
//104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,
//123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,
//142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
//161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
//180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,
//199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,
//218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,
//237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

// permutation table 
int permutation[256] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};


float3 fade(float3 t) {
	return t * t * t * (t * (t * 6 - 15) + 10);// new curve  //  return t * t * (3 - 2 * t); // old curve 
}

float grad(int hash, float x, float y, float z)
{
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	float u = h < 8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
static float Noise(float3 pt)
{
	float3 P;
	P.x = (int)floor(pt.x) & 255;                 // FIND UNIT CUBE THAT CONTAINS POINT.
	P.y = (int)floor(pt.y) & 255;
	P.z = (int)floor(pt.z) & 255;

	pt -= floor(pt);                                // FIND RELATIVE X,Y,Z OF POINT IN CUBE.

	float3 f = fade(pt);                               // COMPUTE FADE CURVES FOR EACH OF X,Y,Z.

	int A = p[P.x] + P.y, AA = p[A] + P.z, AB = p[A + 1] + P.z,      // HASH COORDINATES OF
		B = p[P.x + 1] + P.y, BA = p[B] + P.z, BB = p[B + 1] + P.z;      // THE 8 CUBE CORNERS,

	return float(lerp(lerp(lerp(grad(p[AA], pt.x, pt.y, pt.z),  // AND ADD
		grad(p[BA], pt.x - 1, pt.y, pt.z), f.x), // BLENDED
		lerp(grad(p[AB], pt.x, pt.y - 1, pt.z),  // RESULTS
			grad(p[BB], pt.x - 1, pt.y - 1, pt.z), f.x), f.y),// FROM  8
		lerp(lerp(grad(p[AA + 1], pt.x, pt.y, pt.z - 1),  // CORNERS
			grad(p[BA + 1], pt.x - 1, pt.y, pt.z - 1), f.x), // OF CUBE
			lerp(grad(p[AB + 1], pt.x, pt.y - 1, pt.z - 1),
				grad(p[BB + 1], pt.x - 1, pt.y - 1, pt.z - 1), f.x), f.y), f.z));

}
