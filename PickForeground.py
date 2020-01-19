from PIL import Image
import numpy as np
import cv2
import matplotlib.pyplot as plt


def judgeBackOrFore(H, W, frNum, data, thresholds, index1 ,index2, index3):
    tag = np.zeros([690, 4, 5])
    for frIndex in range(frNum):
        for i in range(H):
            for j in range(W):
                temp = i * H + j
                if frIndex in index1:
                    if data[frIndex][temp] > thresholds[0][i][j]:
                        tag[frIndex][i][j] = 99
                if frIndex in index2:
                    if data[frIndex][temp] > thresholds[1][i][j]:
                        tag[frIndex][i][j] = 99
                if frIndex in index3:
                    if data[frIndex][temp] > thresholds[2][i][j]:
                        tag[frIndex][i][j] = 99

    return tag


def showPictures(H, W, frNum, tag):
    fps = 24
    size = (320, 256)
    video = cv2.VideoWriter("VideoTest1.avi", cv2.VideoWriter_fourcc('I', '4', '2', '0'), fps, size)
    for frIndex in range(frNum):
        imgpath = "media/img/test_images/img_" + str(frIndex).zfill(5) + ".jpg"
        img = np.array(Image.open(imgpath))
        for i in range(H):
            for j in range(W):
                if tag[frIndex][i][j] == 99:
                    for a in range(64):
                        for b in range(64):
                            img[i * 64 + a, j * 64 + b, 0] = img[i * 64 + a, j * 64 + b, 0] * 0.5 + 255 * 0.5
        video.write(img)
    video.release()
    cv2.destroyAllWindows()


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
