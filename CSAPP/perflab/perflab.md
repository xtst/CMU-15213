# rotate函数
考虑分块
http://csapp.cs.cmu.edu/public/waside/waside-blocking.pdf
最后发现8*8的块最快。
注意题目已经开O2了，常见的优化手段比如循环展开没什么意义，只会更慢，毕竟人脑强不过编译器
```
char rotate_descr8[] = "rotate: 8x8 per block";
void rotate(int dim, pixel *src, pixel *dst) {
	int num = dim >> 3;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			int ifrom = (i << 3), jfrom = (j << 3);
			for (int a = 0; a < 8; a++)
				for (int b = 0; b < 8; b++) { dst[RIDX(dim - 1 - jfrom - b, ifrom + a, dim)] = src[RIDX(ifrom + a, jfrom + b, dim)]; }
		}
	}
}
```
# smooth函数
前缀和，入门算法题。
边界情况直接用naive方法的暴力，再套前缀和还要判条件什么的，常数太大。注意把naive方法里的函数展开，太多call会拖慢速度。

```
static pixel avg2(int dim, int i, int j, pixel *src) {
	int ii, jj;
	pixel_sum sum;
	pixel current_pixel;

	sum.red = 0;
	sum.green = 0;
	sum.blue = 0;
	sum.num = 0;
	int low1 = Max(i - 1, 0), up1 = Min(i + 1, dim - 1), low2 = Max(j - 1, 0), up2 = Min(j + 1, dim - 1);
	for (ii = low1; ii <= up1; ii++)
		for (jj = low2; jj <= up2; jj++) {
			sum.red += (int)(src[RIDX(ii, jj, dim)].red);
			sum.green += (int)(src[RIDX(ii, jj, dim)].green);
			sum.blue += (int)(src[RIDX(ii, jj, dim)].blue);
			sum.num++;
		}
	current_pixel.red = (unsigned short)(sum.red / sum.num);
	current_pixel.green = (unsigned short)(sum.green / sum.num);
	current_pixel.blue = (unsigned short)(sum.blue / sum.num);
	return current_pixel;
}

int sum[1027][1027][3];
void smooth(int dim, pixel *src, pixel *dst) {
	int km = dim - 1;
	for (int i = 1; i <= dim; i++)
		for (int j = 1; j <= dim; j++) {
			sum[i][j][0] = sum[i - 1][j][0] + sum[i][j - 1][0] - sum[i - 1][j - 1][0] + src[RIDX(i - 1, j - 1, dim)].red;
			sum[i][j][1] = sum[i - 1][j][1] + sum[i][j - 1][1] - sum[i - 1][j - 1][1] + src[RIDX(i - 1, j - 1, dim)].blue;
			sum[i][j][2] = sum[i - 1][j][2] + sum[i][j - 1][2] - sum[i - 1][j - 1][2] + src[RIDX(i - 1, j - 1, dim)].green;
		}

	for (int i = 1; i < km; i++)
		for (int j = 1; j < km; j++) {
			dst[RIDX(i, j, dim)].red =
				(unsigned int)(sum[i + 2][j + 2][0] - sum[i - 1][j + 2][0] - sum[i + 2][j - 1][0] + sum[i - 1][j - 1][0]) / 9;
			dst[RIDX(i, j, dim)].blue =
				(unsigned int)(sum[i + 2][j + 2][1] - sum[i - 1][j + 2][1] - sum[i + 2][j - 1][1] + sum[i - 1][j - 1][1]) / 9;
			dst[RIDX(i, j, dim)].green =
				(unsigned int)(sum[i + 2][j + 2][2] - sum[i - 1][j + 2][2] - sum[i + 2][j - 1][2] + sum[i - 1][j - 1][2]) / 9;
		}
	for (int i = 0; i < dim; i++) {
		dst[RIDX(i, km, dim)] = avg2(dim, i, km, src);
		dst[RIDX(i, 0, dim)] = avg2(dim, i, 0, src);
	}
	for (int j = 1; j < km; j++) {
		dst[RIDX(0, j, dim)] = avg2(dim, 0, j, src);
		dst[RIDX(km, j, dim)] = avg2(dim, km, j, src);
	}
}
```


# 成绩
```
Summary of Your Best Scores:
  Rotate: 22.4 (rotate: 8x8 per block)
  Smooth: 52.3 (smooth: Current working version)
```