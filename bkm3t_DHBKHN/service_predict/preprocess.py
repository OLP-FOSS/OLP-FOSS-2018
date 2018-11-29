import numpy as np
import json
import pandas as pd
# import utils


def parse_data(raw_data):
    """
    convert from string list of data to array of data
    :param raw_data: string of list dict data :
    [{'yaw': -157.6705, 'pitch': -4.33582, 'roll': -2.451883, 'Ax': 0, 'Ay': 485, 'Az': 7381, 'timestamps': 1542884104.3810017},
    {'yaw': -157.7411, 'pitch': -4.701654, 'roll': -1.870299, 'Ax': -53, 'Ay': 630, 'Az': 7189, 'timestamps': 1542884104.3893933},
    {'yaw': -157.8128, 'pitch': -5.099143, 'roll': -1.229501, 'Ax': -92, 'Ay': 793, 'Az': 6948, 'timestamps': 1542884104.3895116},
    {'yaw': -157.888, 'pitch': -5.491229, 'roll': -0.536322, 'Ax': -101, 'Ay': 972, 'Az': 6662, 'timestamps': 1542884104.3895922},
    {'yaw': -157.9806, 'pitch': -5.876004, 'roll': 0.203376, 'Ax': -78, 'Ay': 1154, 'Az': 6383, 'timestamps': 1542884104.3896697}]
    :return: dataframe of data
    """
    raw_data = raw_data.replace('[', '')
    raw_data = raw_data.replace(']', '')
    raw_data = raw_data.replace('}, {', "};{")
    raw_data = raw_data.replace("'", '"')
    raw_data = raw_data.replace('}\n{', '};{')
    raw_data = raw_data.replace('},{', '};{')
    # print("===================", raw_data)
    lines = raw_data.split(';')

    data = dict({'timestamps': [],
                 'yaw': [],
                 'pitch': [],
                 'roll': [],
                 'ax': [],
                 'ay': [],
                 'az': []})
    for line in lines:
        try:
            line_json = json.loads(line)
            # print(line_json)
            if len(line_json) < 5:
                continue
        except Exception as e:
            print(e)
            continue

        data['timestamps'].append(line_json['timestamps']/1000)
        data['yaw'].append(line_json['yaw'])
        data['pitch'].append(line_json['pitch'])
        data['roll'].append(line_json['roll'])
        data['ax'].append(line_json['Ax'] / 4096)
        data['ay'].append(line_json['Ay'] / 4096)
        data['az'].append(line_json['Az'] / 4096)

    results = pd.DataFrame(data)
    return results


def segment_data_with_timestamp(data, time_split):
    results = list()
    timestamps = data[['timestamps']].values
    # print(timestamps)
    start_time = timestamps[0]
    # if timestamps[-1] - start_time < time_split:
    #     buffer = data
    #     return None
    # if buffer is not None:
    #     data = pd.concat([buffer, data])
    start_index = 0
    for idx, current_time in enumerate(timestamps[1:]):
        delta = current_time - start_time
        if delta >= time_split:
            results.append(data.loc[start_index: idx, ['yaw', 'pitch', 'roll', 'ax', 'ay', 'az']].values)
            start_index = idx + 1
            start_time = timestamps[start_index]
    return results


def segment_data(data, window_size, sample_rate, overlap=0.5):
    """

    :param data:
    :param window_size: seconds
    :param sample_rate: number point per second
    :param overlap: 0 <= x < 1
    :return:
    """
    l = int(window_size * sample_rate)
    num_window = int(len(data) / (l - l * overlap))
    list_data = list()
    for i_w in range(num_window-1):
        i_d = int(i_w * (l - l * overlap))
        window = data[i_d: (i_d + l)].values
        list_data.append(window)
    array_data = np.array(list_data)
    return array_data


def get_features(data):
    """
    :param data: narray segmented data
    :return:
    """
    features = list()
    for window in data:
        mean = np.mean(window, axis=0)
        std = np.std(window, axis=0)
        mad = np.median(window, axis=0)
        min = np.amin(window, axis=0)
        max = np.amax(window, axis=0)
        feature = np.concatenate([mean, std, mad, min, max], axis=0)
        features.append(feature)
    features = np.array(features)
    return features

