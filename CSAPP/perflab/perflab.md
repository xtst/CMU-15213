# rotate函数
考虑分块
http://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf
最后发现16*16的块最快。
注意题目已经开O2了，常见的优化手段比如循环展开没什么意义，只会更慢，毕竟人脑强不过编译器
```
void rotate_16(int dim, pixel *src, pixel *dst) {
	// int k = 0;
	// for (int i = 0; i < dim; i++)
	// for (int j = 0; j < dim; j++) dst[RIDX(dim - 1 - j, i, dim)] = src[k++];

	int num = dim >> 4;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 4), jfrom = (j << 4);
			for (int a = 0; a < 16; a++)
				for (int b = 0; b < 16; b++) { dst[RIDX(dim - 1 - jfrom - b, ifrom + a, dim)] = src[RIDX(ifrom + a, jfrom + b, dim)]; }
		}
	}
}
```
# smooth函数