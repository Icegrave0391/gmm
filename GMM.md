## GMM

### 一. 数据信息

* **基本数据格式**

  `ctu_nums = 20        //每一帧包含20个CTU `

  `frame_nums = 690     //视频片段中包含690frame`

  `shape = (3, 4, 5)    //每一帧分为4x5个ctu, 3代表不同帧的类型 `

* **文件结构**

  1. 帧数据

  `./datalist`文件夹下包含`data1.txt, data2.txt, data3.txt`。

  分别对应奇数帧、被2整除的偶数帧、被4整除的偶数帧。

  <img src="http://q4alsq26d.bkt.clouddn.com/2B713EF9-2FE6-458C-A118-27DC8567CCBB.png" style="zoom:50%;" />

  > 每一行20个数据分为4x5，代表20个CTU
  >
  > lines(data1)+lines(data2)+lines(data3) = 690(frame_nums)

  2. groundtruth

  `./groundtruth`文件夹下包含每一个frame对应的groundtruth.jpg

  3. 视频数据

  `./media/img/test_images`文件夹下对应每一个frame的图片(.jpg)

### 二. 数据处理

数据处理模块：依照文件夹中的数据信息提取有效数据并建立相应数据结构存储。

* **数据结构**

```python
# 所有数据结构第一维均为3，表示分别处理data1.txt, data2.txt, data3.txt
backgroundNum[3,4,5] 												//用于存放相应ctu中所含背景的个数
img_data[frame_num][ctu_num]								//用于存放所有frame中所有ctu的bits数
bg_table[3,4,5][] 													//用于存放相应ctu的所有背景bits数
fg_table[3,4,5][]														//用于存放相应ctu的所有前景bits数
background[3,4,5][(weights, means, sds)]    //用于存放相应ctu经过gmm之后所有分布的参数
```

* **数据处理思路**

将有效信息从`data.txt`以及`groundtruth.jpg`中提取，然后应用高斯混合模型(`n_components = 5`)得出分布。之后按照各分布得到的参数进行training。

#### 1. 提取data.txt中的bits信息

读取data.txt文件，分别提取到list中即可。

```python
#save as list
with open('./datalist/data1.txt', 'r') as f1:
    f_read1 = f1.read()
    f_data1 = f_read1.replace("\n", "").split()
    f_data1 = list(map(int, f_data1))
    f_temp1 = list_of_groups(f_data1, CTU_NUMS)
with open('./datalist/data2.txt', 'r') as f2:
    f_read2 = f2.read()
    f_data2 = f_read2.replace("\n", "").split()
    f_data2 = list(map(int, f_data2))
    f_temp2 = list_of_groups(f_data2, CTU_NUMS)
with open('./datalist/data3.txt', 'r') as f3:
    f_read3 = f3.read()
    f_data3 = f_read3.replace("\n", "").split()
    f_data3 = list(map(int, f_data3))
    f_temp3 = list_of_groups(f_data3, CTU_NUMS)

f_data = [f_data1, f_data2, f_data3]
f_CTU_data = [f_temp1, f_temp2, f_temp3]
```

通过该步骤，三个文件的bits信息分别存放在`f_data=[f_data1, f_data2, f_data3]`中。

提取bits信息如图所示：

![](http://q4alsq26d.bkt.clouddn.com/BE9777F7EA7FBFB52A1C9B48E98870E7.jpg)



#### 2. 通过`matplotlib.image`模块提取groundtruth信息

读取groundtruth.jpg文件，提取信息储存在list中即可。

```python
gt = []
for i in range(1, 690):
    fpath = './groundtruth'+'/gt00'+str(1103+i)+'.png'
    I = mpimg.imread(fpath)
    gt.append(I)
```

通过该步骤，groundtruth的信息被存放在`gt`中。

提取groundtruth如图所示：

<img src="http://q4alsq26d.bkt.clouddn.com/gt.png" style="zoom:50%;" />



#### 3. 储存前景、背景

循环遍历`f_data1, f_data2, f_data3`，根据每一个frame以及每一个ctu在`gt`中的信息判断是否是前景还是背景。将前景和背景的bits分别存放在两个数据结构中。

```python
for i_idx in range(d0):
    data = index[i_idx]
    lens = len(data) #分别处理data1 data2 data3
    for i in range(lens):
        img_data[data[i]] = f_CTU_data[i_idx][i]
        for j in range(d1):
            for k in range(d2):
                frame = data[i]
                temp_jk = gt[frame][j][k]   #对应块的groundtruth
                if temp_jk == 0:            #bg
                    backgroundNum[i_idx][j][k] += 1
                    temp_jkidx = i * CTU_NUMS + j * d2 + k
                    bg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])
                else:
                    temp_jkidx = i * CTU_NUMS + j * d2 + k
                    fg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])
```

经过该步骤，每一个ctu的背景、前景信息分别存放在数据结构中的`bg_table, fg_table`。

提取到的`bg_table`如图所示：

![](http://q4alsq26d.bkt.clouddn.com/bg.png)

提取到的`fg_table`如图所示：

![](http://q4alsq26d.bkt.clouddn.com/fg.jpg)



#### 4. sklearn.GMM高斯混合模型

对于获取到的`bg_table`，分别对第一个维度的三个分量（data1.txt, data2.txt, data3.txt对应的奇数帧、被2整除的偶数帧、被4整除的偶数帧）中的每一个ctu在所有frame中分别进行`n_components=5`的高斯分布。

```python
n_comp = 5
gmm = GaussianMixture(n_components=n_comp, covariance_type='diag', random_state=0)
for i in range(d0):
    for j in range(d1):
        for k in range(d2):
            gmm.fit(np.array(bg_tableFit[i][j][k]).reshape(-1, 1))
            for n in range(n_comp):
                background[i][j][k].append((gmm.weights_[n], float(gmm.means_[n]),float(gmm.covariances_[n])))
```



### 三. 训练threshold

根据数据处理过程中提取到的每一个背景ctu在不同帧上的混合高斯模型，训练threshold。

* *loss function*：

$$
f(thres) = \sum_iw_i*(thres-(\mu_i +3\sigma_i))^2 + (thres-\mu_{F_{min}})^2+(thres-\mu_{B_{max}})^2\\
s.t. thres \in [\sum_iw_i(\mu_i+3\sigma_i),\  \overline{\mu_F}]
$$

> $\mu_i$：背景部分高斯分布的几个均值
>
> $w_i$：背景部分高斯分布的几个权值
>
> $\mu_{F_{min}}$：前景的最小值
>
> $\mu_{B_{max}}$：背景的最大值

让thres取值在约束区间内变动，找到损失函数最小值即可得出最优threshold。

```python
def stepByStep(background, foreground, thresholdIter, end, bMax, fMin, ratio, All, lambda1, step):
    threshold = 0
    lossValue = 99999999
    while thresholdIter < end:
        back = []
        fore = []
        for i in background:
            if i[1] + 3 * i[2] > thresholdIter:
                back.append(i)
        for i in foreground:
            if i < thresholdIter:
                fore.append(i)
        lossValueNew = loss(thresholdIter, back, fore, bMax, fMin, ratio, All, lambda1)
        if lossValueNew < lossValue:
            lossValue = lossValueNew
            threshold = thresholdIter
            thresholdIter = thresholdIter + step
        else:
            thresholdIter = thresholdIter + step
    return threshold


def loss(threshold, back, fore, backMax, foreMin, ratio, All, lambda1):
    lossValue = 0
    for i in back:
        lossValue = lossValue + lambda1 * ratio * i[0] * ((threshold - (i[1] + 3 * i[2])) ** 2)
    for i in fore:
        lossValue = lossValue + (1-lambda1) * (1-ratio) * (1/len(fore)) * (threshold - i)**2
    lossValue = lossValue + 1/All*((threshold-backMax)**2 + (threshold - foreMin)**2)
```



### 四. 当前效果

以ctu(3,3)为例，即第三行第三列的ctu。

* ctu(3,3)在data1、data2、data3下的三个混合高斯模型：

<img src="http://q4alsq26d.bkt.clouddn.com/Figure_1.png" style="zoom:33%;" />

<img src="http://q4alsq26d.bkt.clouddn.com/Figure_2.png" style="zoom:33%;" />

<img src="http://q4alsq26d.bkt.clouddn.com/Figure_3.png" style="zoom:33%;" />

* ctu(3,3)的前背景在data1-3下的比特数频率分布直方图（红色竖线为计算得到的threshold）

<img src="http://q4alsq26d.bkt.clouddn.com/Figure_.png" style="zoom:33%;" />

* 结果的一些分析：

1. gmm的参数

我们观察到：在data1 data2中的高斯混合模型中，很多个高斯分布曲线的高度相同。进一步观察高斯混合模型中的参数如下（分别为ctu(3,3) ctu(4,2)）:

<img src="http://q4alsq26d.bkt.clouddn.com/pa1.jpg" style="zoom:50%;" />

<img src="http://q4alsq26d.bkt.clouddn.com/pa2.jpg" style="zoom:50%;" />

可以看出权重最大的几个高斯分布标准差都为最小标准差$10^{-6}$。即数据分类基本都在几个特定bits的点。

而data3中的bits分布如下(ctu(3,3))：

<img src="http://q4alsq26d.bkt.clouddn.com/pa3.png" style="zoom:50%;" />

可以看出data3的bits过于离散，因此我们需要在数据处理时着重处理。

2. thres的计算和bits的分布

* 特殊宏块ctu(1,5)的数据

通过groundtruth.jpg提取的数据发现宏块ctu(1,5)在690帧中**都不包含前景**。通过公式训练的结果如下：

<img src="http://q4alsq26d.bkt.clouddn.com/15.jpg" style="zoom:40%;" />

发现：不存在前景分布时，公式训练的threshold在频率最高的背景位置。

* 宏块ctu(3,3)的数据

<img src="http://q4alsq26d.bkt.clouddn.com/33.jpg" style="zoom:33%;" />

缩放data1的图：

<img src="http://q4alsq26d.bkt.clouddn.com/33.1.jpg" style="zoom: 50%;" />

<img src="http://q4alsq26d.bkt.clouddn.com/33.2.jpg" style="zoom:33%;" />

发现对于data1的数据，公式有明显偏差（在背景分布的左侧）。对于data3的数据，公式训练的thres又偏大到了前景均值的位置。