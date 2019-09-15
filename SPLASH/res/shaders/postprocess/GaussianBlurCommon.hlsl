cbuffer cbSettings {
	float weights[11] = {
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
	};
};

cbuffer cbFixed {
	static const int blurRadius = 5;
};

#define N 256
#define CacheSize (N + 2*blurRadius)
groupshared float4 cache[cacheSize];