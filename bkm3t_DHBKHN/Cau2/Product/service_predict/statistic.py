import pandas as pd
import matplotlib.pyplot as plt
import utils
from scipy.signal import medfilt, butter

df_acc_train = pd.read_csv('data/dataset_uci/final_acc_test.txt', header=None, names=['ax', 'ay', 'az'])
df_acc_train[:500].plot()
# plt.show()

df = df_acc_train
# df['ax'] = medfilt(df['ax'].values, 19)
# df['ay'] = medfilt(df['ay'].values, 19)
# df['az'] = medfilt(df['az'].values, 19)

df['ax'] = butter(20, df['ax'].values)
df['ay'] = butter(20, df['ay'].values)
df['az'] = butter(20, df['az'].values)

df[:500].plot()
plt.show()

