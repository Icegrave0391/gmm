import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt

def show_CTU_fr(d1, d2, bg_table, fg_table, threshold):

    sub1 = plt.subplot(1,3,1)#data1
    sub1.set_ylabel("frequency")
    sub1.axvline(x=threshold[0][d1][d2], color='#d46061', linewidth=1)
    ctu_bg = np.array(bg_table[0][d1][d2])
    ctu_fg = np.array(fg_table[0][d1][d2])
    sns.distplot(ctu_bg)
    sns.distplot(ctu_fg)

    sub2 = plt.subplot(1,3,2)#data2
    sub2.axvline(x=threshold[1][d1][d2], color='#d46061', linewidth=1)
    ctu_bg = np.array(bg_table[1][d1][d2])
    ctu_fg = np.array(fg_table[1][d1][d2])
    sns.distplot(ctu_bg)
    sns.distplot(ctu_fg)

    sub3 = plt.subplot(1, 3, 3)  # data3
    sub3.axvline(x=threshold[2][d1][d2], color='#d46061', linewidth=1)
    ctu_bg = np.array(bg_table[2][d1][d2])
    ctu_fg = np.array(fg_table[2][d1][d2])
    sns.distplot(ctu_bg)
    sns.distplot(ctu_fg)
    plt.show()