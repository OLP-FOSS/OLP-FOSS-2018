import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix

lbl = {
    1: 'WALKING',
    2: 'WALKING_UPSTAIRS',
    3: 'WALKING_DOWNSTAIRS',
    4: 'SITTING',
    5: 'STANDING',
    6: 'LAYING',
    7: 'STAND_TO_SIT',
    8: 'SIT_TO_STAND',
    9: 'SIT_TO_LIE',
    10: 'LIE_TO_SIT',
    11: 'STAND_TO_LIE',
    12: 'LIE_TO_STAND'
}


def show_confusion_matrix(true_label, predict, file_save):
    conf_mat = confusion_matrix(true_label, predict)

    fig = plt.figure(figsize=(12, 12))
    width = np.shape(conf_mat)[1]
    height = np.shape(conf_mat)[0]

    res = plt.imshow(np.array(conf_mat), cmap=plt.cm.GnBu, interpolation='nearest')
    for i, row in enumerate(conf_mat):
        for j, c in enumerate(row):
            if c > 0:
                plt.text(j - .2, i + .1, c, fontsize=16)

    cb = fig.colorbar(res)
    plt.title('Confusion Matrix')
    _ = plt.xticks(range(12), [l for l in lbl.values()], rotation=90)
    _ = plt.yticks(range(12), [l for l in lbl.values()])
    plt.savefig(file_save)