# VCX-lab2

### Task 1: Loop Mesh Subdivision

该lab里仅针对封闭的三维几何体进行模型细分。

#### 实现思路

##### (1) 计算新增点坐标

<img src="./report_image/loop_subdivision_1.png" alt="Loop Subdivision" style="zoom: 50%;" />

对于每一条边，需要按照如上图左的比例，增加一个新的顶点。

实现时，利用提供的```DCEL```数据结构，枚举每一条边，获取该边的两个三角面，将两个三角面的顶点相加，再加上该边的两个顶点，最后除以8，即可得到新点的坐标。
$$
(V_0+V_1+V_2)+(V_3+V_1+V_2)+(V_1+V_2) = \frac 1 8 V_0+\frac 3 8 V_1 + \frac 3 8 V_2+ \frac 1 8 V_3
$$
直接将新点加入```SurfaceMesh```中，使用一个```unordered_map```来保存边到该点的映射关系。

由于```DCEL```中包含一条边的两个方向，为了防止重复增加点，需要使用```unordered_map```来判断一下该点是否已被添加。

##### (2) 修改旧顶点坐标

对于旧顶点，需要按照上图右的比例，修改。

实现时，使用顶点坐标数组```sum```存储每个旧顶点周围的新顶点坐标和，```deg```数组存储该顶点的度数。该步骤可以在计算新增点坐标时更新完成。

然后，更新旧顶点坐标，使用如下公式即可（其中u按照图中公式计算）。
$$
V_i = (1-u\ \text{deg}[i])V_i+u\ \text{sum}[i]
$$

##### (3) 连接新顶点，设置Indices

使用```DCEL```数据结构枚举每一个旧的三角面，通过```unordered_map```获取他每条边上新增点的编号，得到下图。

<img src="./report_image/loop_subdivision_2.png" alt="loop_subdivision_2" style="zoom:60%;" />

向```SurfaceMesh```的```Indices```添加序列$v_0e_2e_1,v_1e_0e_2,v_2e_1e_0,e_0e_1e_2$即可



重复以上步骤```numIteration```次

#### 注意事项

在```DCEL```的实现中，使用了```unordered_map```，且用的hash值类型为```std::size_t```，在计算hash过程中，使用了左移32位的操作。

在32位系统下，```std::size_t```为32位的，左移32位操作会出现溢出，导致hash出错，bug极难察觉。

#### 效果

![loop_subdivision_result](./report_image/loop_subdivision_3.png)

### Task 2: Spring-Mass Mesh Parameterization

根据算法推导，最终希望得到满足如下条件的顶点UV坐标 $t$. 且边界上的点在正方形$[0,1]^2$的边界上平均分配
$$
t_i=\sum_{j\in N_i}\lambda_{ij}t_j
$$
$N_i$为结点$i$的邻居集合；对于系数$\lambda_{ij}$，简单地选择$\lambda_{ij}=\frac 1 {|N_i|}$

#### 实现思路

首先通过```DCEL```，找到一个在边界上的顶点。通过```DCEL```，将整个边界上的一圈顶点按顺序存储在数组```side_v```中

将```side_v```中的点分成4段，每一段将其材质UV均匀的放在正方形的一条边上

迭代```numIterations```次，每次枚举所有顶点，通过以下公式更新该顶点的UV坐标
$$
t_i=\frac 1 {|N_i|}\sum_{j\in N_i}t_j
$$
经过许多次迭代，更新的幅度会越来越小，趋于稳定。

#### 效果

![Spring-Mass Mesh Parameterization](./report_image/spring_parameterization_1.png)

