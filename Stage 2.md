## Stage 2

1. 数据提取部分。只使用data1.txt的数据作为计算threshold的依据（计算出的threshold表示该奇数帧和下一个偶数帧的threshold）
2. threshold计算部分。不需要精确将threshold界定在刚刚好的最大背景bits上，允许一定范围的冗余（为了确保分组加密后thres依旧不会误判背景）。有两种思路：
   * 仍旧使用GMM，选择$\mu+3\sigma$最大的那个分布得到的结果作为threshold
   * 不使用GMM，直接通过提取到的数据进行去噪（个人感觉data1没必要去噪），在数据中选择bits最大的作为threshold

3. 将计算得到的threshold，应用于该帧和下一个紧接着的偶数帧。