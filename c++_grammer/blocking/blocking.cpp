// blocking!!!
// using a block-based approach -> reducing the size of working set 
// -> reducing cache misses -> improving memory access

// 外层循环确定working block
// 内层循环在该working block内执行相应工作

for(int i=0;i<=N-block;i+=block)
	for(int j=0;j<=N-block;j+=block)
		for(int a=i;a<i+block;a++)
			for(int b=j;b<j+block;b++)
				// 这里的例子是是使用矩阵转置
				dst[b*N+a] = src[a*N +b]


//处理不能被包含到整块里的数据
// i应该定义在外面 不然括号外就看不到了
int offset = i; // 因为是正方形最后下标i, j停在了相同的位置 i==j
for(int i=offset;i<N;i++)
	for(int j=0;j<offset;j+=block)
		for(int b=j;b<j+block;b++)
			dst[b*N+i] = src[i*N+b];


for(int i=0;i<N;i++)
	for(int j=offset;j<N;j++)
		dst[j*N+i] = src[i*N+j];



// 另一个需要注意的问题 -> 将二维数组(NxN)转换为一维数组
// matrix[i][j] -> matrix[i*N +j] 

// 一个应用 -> 将一个有向图g转换成它对应的无向图g' 
// 运用分块技术 以块为单位进行修改	-> 分块和处理块外数据的结构并不改变 -> 只修改具体执行的操作

// 函数原型
void fun(int *G, int N, int block);


for(int i=0;i<=N-blcok;i+=block)
	for(int j=i;j<=N-block;j+=block)
		for(int a=i;a<i+block;a++)
			for(int b=j;b<j+block;b++)
			{
				int temp = G[b*N+a] || G[a*N+b];
				G[b*N+a] = temp;
				G[a*N+b] = temp;
			}


int offset = i;
for(int i=offset;i<N;i++)
	// offset是没有作为块的i的最后迭代的值 -> 此处没有等号 -> 下一步会取到
	for(int j=0;j<offset;j+=block)
		for(int b=j;b<j+block;b++)
			{
				int temp = G[b*N+i] || G[i*N+b];
				G[i*N+b] = temp;
				G[b*N+i] = temp;
			}

for(int i=offset;i<N;i++)
	for(int j=i;j<N;j++)
			{
				int temp = G[j*N+i] || G[i*N+j];
				G[i*N+j] = temp;
				G[j*N+i] = temp;
			}

// 标准答案 -> 因为对称性 这道题做了少许改进 只处理一半的块和一半的块外数据

