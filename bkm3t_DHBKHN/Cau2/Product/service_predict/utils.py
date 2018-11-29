import numpy as np


def magnitude(x, y, z):
    """
    Magnitude signal computed from the previous three signals.
    This magnitude is computed as the square root of the sum
    of squared components (accelerometer signals).
    :param ax:
    :param ay:
    :param az:
    :return:
    """
    x2 = np.square(x)
    y2 = np.square(y)
    z2 = np.square(z)
    sum = np.sum([x2, y2, z2], axis=0)
    return np.sqrt(sum)


def jerk(acc, time):
    result = np.zeros(len(acc))
    for i, value in enumerate(acc[1:]):
        da = value - acc[i]
        dt = time[i+1] - time[i]
        result[i + 1] = da / dt
    return result


def fft(array):
    """
    Fast Fourier transforms (FFTs)
    :param array:
    :return:
    """
    return np.fft.fft(array)


def sma(x, y=None, z=None):
    if y is None and z is None:
        return np.mean(np.abs(x))
    else:
        return np.mean(np.abs(x) + np.abs(y) + np.abs(z))

