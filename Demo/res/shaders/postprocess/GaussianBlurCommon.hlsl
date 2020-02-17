static const int blurRadius = 2;

static float weights[blurRadius*2+1] = {
	0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136f
};

#define N 256
#define cacheSize (N + 2 * blurRadius)
groupshared float4 cache[cacheSize];