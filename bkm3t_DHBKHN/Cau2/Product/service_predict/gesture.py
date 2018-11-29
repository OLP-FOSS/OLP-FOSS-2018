import numpy as np
import pandas as pd
from dtwknn import DtwKnn

min_window_size = 30
max_window_size = 100
threshold_mean = 0
threshold_std = 0.5
threshold_change = 1

model = DtwKnn(n_neighbors=1)


def segment_window(data, array, min_window_size, max_window_size):
    index_segment = list()
    prev_point = array[0]
    start_index_window = 0
    for i, value in enumerate(array[1:]):
        if value >= 0 > prev_point or value < 0 <= prev_point:
            if max_window_size >= i - start_index_window >= min_window_size:
                # print(start_index_window, i)
                index_segment.append((start_index_window, i))
                start_index_window = i
        prev_point = value

    result = list()
    for value in index_segment:
        window = data[value[0]:value[1]].values
        result.append(window)
    return result


def get_features(data):
    features = list()
    for window in data:
        window = window[:, 1:]
        print(len(window))
        max_win = np.amax(window, axis=0)
        min_win = np.amin(window, axis=0)
        mean_win = np.mean(window, axis=0)
        std_win = np.std(window, axis=0)
        mad_win = np.median(window, axis=0)
        feature = np.concatenate([max_win, min_win, mean_win, std_win, mad_win], axis=0)
        features.append(feature)
    return np.array(features)


def get_gesture(feature_window, field):
    mapping = {
        'ax': [15, 21, 'left', 'right'],
        'ay': [16, 22, 'left', 'right'],
        'az': [17, 23, 'up', 'down']
    }
    print(feature_window[15:24], field)
    if feature_window[mapping[field][1]] < threshold_std:
        return None

    if feature_window[mapping[field][0]] > threshold_mean:
        return mapping[field][2]
    else:
        return mapping[field][3]


def get_gestures(data):
    data_segmented_x = segment_window(data, data[['ax']].values,
                                      min_window_size=min_window_size, max_window_size=max_window_size)
    data_segmented_y = segment_window(data, data[['ay']].values,
                                      min_window_size=min_window_size, max_window_size=max_window_size)
    data_segmented_z = segment_window(data, data[['az']].values,
                                      min_window_size=min_window_size, max_window_size=max_window_size)

    features_x = get_features(data_segmented_x)
    features_y = get_features(data_segmented_y)
    features_z = get_features(data_segmented_z)

    predictions_x = list()
    predictions_y = list()
    predictions_z = list()

    for feature in features_x:
        predict = get_gesture(feature, 'ax')
        if predict is not None:
            predictions_x.append(predict)
    for feature in features_y:
        predict = get_gesture(feature, 'ay')
        if predict is not None:
            predictions_y.append(predict)
    for feature in features_z:
        predict = get_gesture(feature, 'az')
        if predict is not None:
            predictions_z.append(predict)

    print(predictions_x, predictions_y, predictions_z)
    if len(predictions_z) > 0:
        return predictions_z[0]
    elif len(predictions_y) > 0:
        return predictions_y[0]
    elif len(predictions_x) > 0:
        return predictions_x[0]
    # return predictions_x, predictions_y, predictions_z


def get_gestures(data):
    ax = data[['ax']].values
    ay = data[['ay']].values
    az = data[['az']].values

    ax_change = np.amax(ax) - np.amin(ax)
    ay_change = np.amax(ay) - np.amin(ay)
    az_change = np.amax(az) - np.amin(az)

    if ax_change > ay_change and ax_change > az_change:
        if ax_change > threshold_change:
            sequence_max_min = np.argmax(ax) - np.argmin(ax)
            if sequence_max_min > 0:
                return 'in'
            else:
                return 'out'
        else:
            return 'fixedly'
    elif ay_change > ax_change and ay_change > az_change:
        if ay_change > threshold_change:
            sequence_max_min = np.argmax(ay) - np.argmin(ay)
            if sequence_max_min > 0:
                return 'left'
            else:
                return 'right'
        else:
            return 'fixedly'

    elif az_change > ax_change and az_change > ay_change:
        if az_change > threshold_change:
            sequence_max_min = np.argmax(az) - np.argmin(az)
            if sequence_max_min > 0:
                return 'down'
            else:
                return 'up'
        else:
            return 'fixedly'


def get_gesture(data):
    return model.predict(data[['ax', 'ay', 'az']].values)


def nomalize(data):
    if data.shape[1] != 3:
        return data
    mean = np.mean(data, axis=1)
    print(mean)
    result = np.copy(data)
    for i in range(len(data)):
        result[i] = (data[i] - mean[i]) **2

    return result


import preprocess
data = "{'yaw': -156.3455, 'pitch': -4.241972, 'roll': 5.05689, 'Ax': -112, 'Ay': 1287, 'Az': 4291, 'timestamps': 1542884105.557945}," \
       "{'yaw': -156.0678, 'pitch': -4.079327, 'roll': 3.900372, 'Ax': -122, 'Ay': 1113, 'Az': 4262, 'timestamps': 1542884105.5580056}," \
       "{'yaw': -155.8473, 'pitch': -3.816458, 'roll': 2.781065, 'Ax': 55, 'Ay': 1054, 'Az': 4229, 'timestamps': 1542884105.6002154}," \
       "{'yaw': -155.6672, 'pitch': -3.428318, 'roll': 1.740474, 'Ax': 86, 'Ay': 906, 'Az': 4343, 'timestamps': 1542884105.6002948},," \
"{'yaw': -155.5444, 'pitch': -3.000628, 'roll': 0.804388, 'Ax': 28, 'Ay': 754, 'Az': 4623, 'timestamps': 1542884105.6003606}," \
"{'yaw': -155.3859, 'pitch': -2.705737, 'roll': -0.067157, 'Ax': -147, 'Ay': 571, 'Az': 4985, 'timestamps': 1542884105.6412446}," \
"{'yaw': -155.1585, 'pitch': -2.556782, 'roll': -0.87865, 'Ax': -336, 'Ay': 386, 'Az': 5219, 'timestamps': 1542884105.641341}," \
"{'yaw': -154.9243, 'pitch': -2.484698, 'roll': -1.650262, 'Ax': -445, 'Ay': 82, 'Az': 5294, 'timestamps': 1542884105.6414068}," \
"{'yaw': -154.6765, 'pitch': -2.404367, 'roll': -2.377833, 'Ax': -455, 'Ay': 140, 'Az': 5245, 'timestamps': 1542884105.687}," \
"{'yaw': -154.4199, 'pitch': -2.318501, 'roll': -3.018516, 'Ax': -307, 'Ay': -40, 'Az': 5212, 'timestamps': 1542884105.6870792}," \
"{'yaw': -154.2378, 'pitch': -2.198331, 'roll': -3.678558, 'Ax': -158, 'Ay': -30, 'Az': 5371, 'timestamps': 1542884105.6871283}," \
"{'yaw': -154.093, 'pitch': -2.132689, 'roll': -4.314784, 'Ax': -84, 'Ay': -91, 'Az': 5683, 'timestamps': 1542884105.6871827}," \
"{'yaw': -153.9748, 'pitch': -2.107752, 'roll': -4.89504, 'Ax': -66, 'Ay': -117, 'Az': 6052, 'timestamps': 1542884105.68723}," \
"{'yaw': -153.8799, 'pitch': -2.149193, 'roll': -5.404916, 'Ax': -152, 'Ay': -193, 'Az': 6413, 'timestamps': 1542884105.726883}," \
"{'yaw': -153.7673, 'pitch': -2.243523, 'roll': -5.834291, 'Ax': -245, 'Ay': -260, 'Az': 6756, 'timestamps': 1542884105.7270029}," \
"{'yaw': -153.6616, 'pitch': -2.38728, 'roll': -6.203962, 'Ax': -336, 'Ay': -327, 'Az': 7005, 'timestamps': 1542884105.7270916}," \
"{'yaw': -153.5626, 'pitch': -2.56391, 'roll': -6.524476, 'Ax': -411, 'Ay': -384, 'Az': 7128, 'timestamps': 1542884105.72718}," \
"{'yaw': -153.4549, 'pitch': -2.757556, 'roll': -6.799609, 'Ax': -444, 'Ay': -433, 'Az': 7191, 'timestamps': 1542884105.7589316}," \
"{'yaw': -153.3611, 'pitch': -2.954507, 'roll': -7.032735, 'Ax': -415, 'Ay': -463, 'Az': 7234, 'timestamps': 1542884105.759093}," \
"{'yaw': -153.285, 'pitch': -3.150198, 'roll': -7.214338, 'Ax': -337, 'Ay': -435, 'Az': 7284, 'timestamps': 1542884105.7592258}," \
"{'yaw': -153.255, 'pitch': -3.344823, 'roll': -7.29304, 'Ax': -259, 'Ay': -403, 'Az': 7365, 'timestamps': 1542884105.759324}," \
"{'yaw': -153.2473, 'pitch': -3.560858, 'roll': -7.260257, 'Ax': -154, 'Ay': -352, 'Az': 7457, 'timestamps': 1542884105.8011692}," \
"{'yaw': -153.2546, 'pitch': -3.767433, 'roll': -7.122837, 'Ax': -95, 'Ay': -330, 'Az': 7515, 'timestamps': 1542884105.801246}," \
"{'yaw': -153.3003, 'pitch': -3.947293, 'roll': -6.897265, 'Ax': -49, 'Ay': -273, 'Az': 7507, 'timestamps': 1542884105.8013172}," \
"{'yaw': -153.3622, 'pitch': -4.09703, 'roll': -6.59056, 'Ax': -15, 'Ay': -211, 'Az': 7502, 'timestamps': 1542884105.8013637}," \
"{'yaw': -153.45, 'pitch': -4.208388, 'roll': -6.208128, 'Ax': -7, 'Ay': -143, 'Az': 7491, 'timestamps': 1542884105.8456578}," \
"{'yaw': -153.5487, 'pitch': -4.320714, 'roll': -5.760128, 'Ax': -10, 'Ay': -53, 'Az': 7478, 'timestamps': 1542884105.8457532}," \
"{'yaw': -153.6823, 'pitch': -4.473983, 'roll': -5.262888, 'Ax': -55, 'Ay': 20, 'Az': 7499, 'timestamps': 1542884105.8458283}," \
"{'yaw': -153.8109, 'pitch': -4.686001, 'roll': -4.721484, 'Ax': -148, 'Ay': 100, 'Az': 7509, 'timestamps': 1542884105.8830402}," \
"{'yaw': -153.931, 'pitch': -4.948474, 'roll': -4.142074, 'Ax': -247, 'Ay': 216, 'Az': 7477, 'timestamps': 1542884105.8831162}," \
"{'yaw': -154.0307, 'pitch': -5.245, 'roll': -3.498677, 'Ax': -330, 'Ay': 366, 'Az': 7387, 'timestamps': 1542884105.8831632}," \
"{'yaw': -154.1275, 'pitch': -5.569891, 'roll': -2.818205, 'Ax': -371, 'Ay': 534, 'Az': 7208, 'timestamps': 1542884105.8832061}," \
"{'yaw': -154.2133, 'pitch': -5.910738, 'roll': -2.090694, 'Ax': -364, 'Ay': 703, 'Az': 6916, 'timestamps': 1542884105.883249}," \
"{'yaw': -154.2964, 'pitch': -6.262923, 'roll': -1.336165, 'Ax': -306, 'Ay': 849, 'Az': 6530, 'timestamps': 1542884105.9175203}," \
"{'yaw': -154.3927, 'pitch': -6.61665, 'roll': -0.565788, 'Ax': -199, 'Ay': 990, 'Az': 6115, 'timestamps': 1542884105.9175963}," \
"{'yaw': -154.524, 'pitch': -6.936921, 'roll': 0.229882, 'Ax': -45, 'Ay': 1140, 'Az': 5734, 'timestamps': 1542884105.9176493}," \
"{'yaw': -154.7124, 'pitch': -7.211789, 'roll': 1.069079, 'Ax': 109, 'Ay': 1299, 'Az': 5393, 'timestamps': 1542884105.917706}," \
"{'yaw': -154.9322, 'pitch': -7.447845, 'roll': 1.948438, 'Ax': 189, 'Ay': 1447, 'Az': 5133, 'timestamps': 1542884105.9517033}," \
"{'yaw': -155.1788, 'pitch': -7.686025, 'roll': 2.865968, 'Ax': 157, 'Ay': 1560, 'Az': 4930, 'timestamps': 1542884105.9517903}," \
"{'yaw': -155.4162, 'pitch': -7.94449, 'roll': 3.801454, 'Ax': 27, 'Ay': 1632, 'Az': 4731, 'timestamps': 1542884105.9518454}," \
"{'yaw': -155.6209, 'pitch': -8.23939, 'roll': 4.757851, 'Ax': -124, 'Ay': 1689, 'Az': 4527, 'timestamps': 1542884105.9518948}," \
"{'yaw': -155.7849, 'pitch': -8.561288, 'roll': 5.722372, 'Ax': -226, 'Ay': 1750, 'Az': 4275, 'timestamps': 1542884105.9877293}," \
"{'yaw': -155.9329, 'pitch': -8.879982, 'roll': 6.724547, 'Ax': -203, 'Ay': 1817, 'Az': 3954, 'timestamps': 1542884105.9878056}," \
"{'yaw': -156.0989, 'pitch': -9.155292, 'roll': 7.752684, 'Ax': -111, 'Ay': 1858, 'Az': 3580, 'timestamps': 1542884105.9878633}," \
"{'yaw': -156.2776, 'pitch': -9.383136, 'roll': 8.785131, 'Ax': -26, 'Ay': 1890, 'Az': 3216, 'timestamps': 1542884106.0379453}," \
"{'yaw': -156.4756, 'pitch': -9.57201, 'roll': 9.798349, 'Ax': 76, 'Ay': 1937, 'Az': 2909, 'timestamps': 1542884106.038025}," \
"{'yaw': -156.7124, 'pitch': -9.736346, 'roll': 10.80538, 'Ax': 147, 'Ay': 2005, 'Az': 2710, 'timestamps': 1542884106.0380743}," \
"{'yaw': -156.9758, 'pitch': -9.862908, 'roll': 11.84362, 'Ax': 137, 'Ay': 2062, 'Az': 2520, 'timestamps': 1542884106.0381222}," \
"{'yaw': -157.2204, 'pitch': -9.980339, 'roll': 12.92673, 'Ax': 84, 'Ay': 2121, 'Az': 2310, 'timestamps': 1542884106.0638685}," \
"{'yaw': -157.47, 'pitch': -10.07106, 'roll': 14.05175, 'Ax': 46, 'Ay': 2133, 'Az': 2107, 'timestamps': 1542884106.0639646}," \
"{'yaw': -157.7104, 'pitch': -10.17557, 'roll': 15.16724, 'Ax': -19, 'Ay': 2103, 'Az': 1894, 'timestamps': 1542884106.0640323}," \
"{'yaw': -157.8882, 'pitch': -10.32674, 'roll': 16.25119, 'Ax': -104, 'Ay': 2020, 'Az': 1669, 'timestamps': 1542884106.1082673}," \
"{'yaw': -158.0443, 'pitch': -10.49093, 'roll': 17.33448, 'Ax': -124, 'Ay': 1951, 'Az': 1379, 'timestamps': 1542884106.1083612}," \
"{'yaw': -158.2106, 'pitch': -10.60443, 'roll': 18.4781, 'Ax': -49, 'Ay': 1849, 'Az': 1021, 'timestamps': 1542884106.108435}," \
"{'yaw': -158.4131, 'pitch': -10.61681, 'roll': 19.60341, 'Ax': -6, 'Ay': 1698, 'Az': 712, 'timestamps': 1542884106.1085036}," \
"{'yaw': -158.6052, 'pitch': -10.58704, 'roll': 20.64645, 'Ax': -56, 'Ay': 1591, 'Az': 579, 'timestamps': 1542884106.108564}," \
"{'yaw': -158.7882, 'pitch': -10.57035, 'roll': 21.61422, 'Ax': -134, 'Ay': 1544, 'Az': 527, 'timestamps': 1542884106.1416821}," \
"{'yaw': -158.9778, 'pitch': -10.56098, 'roll': 22.52459, 'Ax': -276, 'Ay': 1464, 'Az': 544, 'timestamps': 1542884106.141778}," \
"{'yaw': -159.1028, 'pitch': -10.61171, 'roll': 23.44703, 'Ax': -412, 'Ay': 1445, 'Az': 572, 'timestamps': 1542884106.1418417}," \
"{'yaw': -159.215, 'pitch': -10.70039, 'roll': 24.43924, 'Ax': -500, 'Ay': 1383, 'Az': 528, 'timestamps': 1542884106.1795347}," \
"{'yaw': -159.3171, 'pitch': -10.75219, 'roll': 25.45008, 'Ax': -564, 'Ay': 1311, 'Az': 422, 'timestamps': 1542884106.1796298}," \
"{'yaw': -159.4058, 'pitch': -10.78391, 'roll': 26.39867, 'Ax': -627, 'Ay': 1265, 'Az': 359, 'timestamps': 1542884106.179692}," \
"{'yaw': -159.4763, 'pitch': -10.83352, 'roll': 27.27213, 'Ax': -682, 'Ay': 1250, 'Az': 315, 'timestamps': 1542884106.179751}," \
"{'yaw': -159.5251, 'pitch': -10.89677, 'roll': 28.11562, 'Ax': -689, 'Ay': 1227, 'Az': 261, 'timestamps': 1542884106.212718}," \
"{'yaw': -159.5712, 'pitch': -10.9736, 'roll': 28.96612, 'Ax': -656, 'Ay': 1162, 'Az': 186, 'timestamps': 1542884106.2128263}," \
"{'yaw': -159.61, 'pitch': -11.07466, 'roll': 29.80674, 'Ax': -618, 'Ay': 1077, 'Az': 104, 'timestamps': 1542884106.2128975}," \
"{'yaw': -159.6298, 'pitch': -11.20349, 'roll': 30.62563, 'Ax': -571, 'Ay': 1002, 'Az': 9, 'timestamps': 1542884106.2547386}," \
"{'yaw': -159.6385, 'pitch': -11.34169, 'roll': 31.41361, 'Ax': -514, 'Ay': 926, 'Az': -92, 'timestamps': 1542884106.2548318}," \
"{'yaw': -159.6405, 'pitch': -11.45898, 'roll': 32.17553, 'Ax': -445, 'Ay': 869, 'Az': -208, 'timestamps': 1542884106.254883}," \
"{'yaw': -159.6502, 'pitch': -11.55294, 'roll': 32.91998, 'Ax': -372, 'Ay': 870, 'Az': -281, 'timestamps': 1542884106.2549314}," \
"{'yaw': -159.6713, 'pitch': -11.62751, 'roll': 33.64515, 'Ax': -336, 'Ay': 906, 'Az': -286, 'timestamps': 1542884106.2921698}," \
"{'yaw': -159.7033, 'pitch': -11.69278, 'roll': 34.36755, 'Ax': -354, 'Ay': 924, 'Az': -247, 'timestamps': 1542884106.292353}," \
"{'yaw': -159.7199, 'pitch': -11.78463, 'roll': 35.07945, 'Ax': -421, 'Ay': 877, 'Az': -211, 'timestamps': 1542884106.2924182}," \
"{'yaw': -159.7213, 'pitch': -11.88388, 'roll': 35.76874, 'Ax': -486, 'Ay': 824, 'Az': -220, 'timestamps': 1542884106.2924783}," \
"{'yaw': -159.685, 'pitch': -12.01974, 'roll': 36.41698, 'Ax': -522, 'Ay': 755, 'Az': -247, 'timestamps': 1542884106.3296063}," \
"{'yaw': -159.634, 'pitch': -12.16222, 'roll': 37.0293, 'Ax': -505, 'Ay': 698, 'Az': -318, 'timestamps': 1542884106.3297029}"
# a = preprocess.parse_data(data)
# a = a[['ax', 'ay', 'az']].values
# import matplotlib.pyplot as plt
#
# plt.plot(a[:, 0])
# plt.plot(a[:, 1])
# plt.plot(a[:, 2])
#
# # plt.show()
#
#
# b = nomalize(a)
# plt.plot(b[:, 0])
# plt.plot(b[:, 1])
# plt.plot(b[:, 2])
#
# plt.legend(['ax1', 'ay1', 'az1', 'ax2', 'ay2', 'az2'])
# plt.show()