static const int blurRadius = 2;

// Sigma 2.0
static float weights[blurRadius*2+1] = {
	0.153388f, 0.221461f, 0.250301f, 0.221461f, 0.153388f
};

#define N 256
#define cacheSize (N + 2 * blurRadius)
groupshared float4 cache[cacheSize];