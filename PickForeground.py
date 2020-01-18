from PIL import Image
import numpy as np
import matplotlib.pyplot as plt


def judgeBackOrFore(H, W, frNum, data, thresholds, ):
    tag = np.zeros([690, 4, 5])
    for frIndex in range(frNum):
        for i in range(H):
            for j in range(W):
                if data[frNum][i][j] > thresholds[i][j]:
                    tag[frNum][i][j] = 99
    return tag


def showPictures(H, W, frNum, tag):
    for frIndex in range(frNum):
        imgpath = "./media/img/test_images/img_" + str(frIndex).zfill(5) + ".jpg"
        img = np.array(Image.open(imgpath))
        for i in H:
            for j in W:
                if tag[frIndex][i][j] == 99:
                    for a in range(64):
                        for b in range(64):
                            img[i * 64 + a, j * 64 + b, 0] = img[i * 64 + a, j * 64 + b, 0] * 0.5 + 255 * 0.5
        plt.imshow(img)
        plt.axis('off')
        plt.show()


if __name__ == "__main__":
    imgpath = "test.jpg"
    img = np.array(Image.open(imgpath))
    rows, cloms, dims = img.shape
    H = rows//64
    W = cloms//64
    for i in range(H):
        for j in range(W):
            if (i+j) % 2 == 0:
                for a in range(64):
                    for b in range(64):
                        img[i * 64 + a, j * 64 + b, 0] = img[i * 64 + a, j * 64 + b, 0] * 0.5 + 255 * 0.5

    plt.imshow(img)
    plt.axis('off')
    plt.show()
