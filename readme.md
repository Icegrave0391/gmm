## Readme

### 数据格式与分布

* **groundtruth**

文件在`/groundtruth`下

* **data**

所有文件在`/datalist`下，分为data1.txt，data2.txt，data3.txt。

>  回顾一下比特数的提取过程：将一帧分为4x5的20个块，分别对每一块提取。同时将帧分为**奇数帧**、**被2整除的偶数帧**、**被4整除的偶数帧**。
>
> 由此得到data1.txt, data2.txt, data3.txt

<img src="https://res.cloudinary.com/dg6z7pidk/image/upload/v1578915324/gmmreadme/5B2CC228-F4A0-4EDE-9664-59CE0CA1F94C_zcbzy5.jpg" style="zoom:33%;" />

以data1.txt为例，每行20个数据（将一帧按照4x5划分），每行对应一帧。



### 当前处理结果分析

* 在一个特定CTU上进行GMM处理的结果：

![](https://res.cloudinary.com/dg6z7pidk/image/upload/v1578915729/gmmreadme/IMG_0896_z5jzx0.png)

上图totalbits：横坐标代表ctu(4,2)对应每一帧的总bits，红色是背景、绿色是前景。

下图：

![](https://res.cloudinary.com/dg6z7pidk/image/upload/v1578915506/gmmreadme/66E53635-D7E1-4469-9F04-FF495B2D4D0D_gasrk7.jpg)

> 红色、绿色分别是通过groundtruth获取的背景、前景。
>
> **横坐标**：比特数
>
> **纵坐标**：该比特数在所有帧中的频率
>
> **绿色曲线：**对于背景运用GMM得到的一个曲线

threshold：特定曲线的$\mu + 3\sigma$，对于是哪一条后文会指出。

* GMM曲线

<img src="https://res.cloudinary.com/dg6z7pidk/image/upload/v1578915729/gmmreadme/IMG_0897_scdktj.png"  />

可以看出GMM是下图五个高斯分布。

* 得出threshold的方式：

![](https://res.cloudinary.com/dg6z7pidk/image/upload/v1578915729/gmmreadme/IMG_0898_g4pwdz.png)

可见当前得出threshold方式是：取5个高速分布中通过系数运算最大的那个。



### 疑问&需要解决的问题

**需要解决：**

当前threshold设置明显不合理，因为threshold设置的过小导致很多背景被当作前景。



**疑问：**

为什么threshold的得出方式是上文描述的方式，而且得到的还比真实值偏小？

分析应该是用按权得到的gmm高斯分布系数计算，但是当前的计算方法得到的threshold居然还偏小？







