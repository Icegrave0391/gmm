## Readme

### 数据格式与分布

* **groundtruth**

文件在`/groundtruth`下

* **data**

所有文件在`/datalist`下，分为data1.txt，data2.txt，data3.txt。

>  回顾一下比特数的提取过程：将一帧分为4x5的20个块，分别对每一块提取。同时将帧分为**奇数帧**、**被2整除的偶数帧**、**被4整除的偶数帧**。
>
> 由此得到data1.txt, data2.txt, data3.txt