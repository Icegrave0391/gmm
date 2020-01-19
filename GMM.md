## GMM

### 数据信息

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

### 数据处理

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

