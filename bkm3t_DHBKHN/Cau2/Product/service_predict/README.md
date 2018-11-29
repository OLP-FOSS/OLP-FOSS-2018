# olp_smartwatch

## Install dependencies
 - Install python 3
 - Install package dependencies:
    ```shell
    pip install -r requirements.txt
    ```
    
## Run service
    python service.py
    

## gesture
- link: localhost:5000/predict_gesture
- method: post
- data: list points of data, each point has yaw, pitch, roll, ax, ay, ax, timestamp
- number of data: about 30 point
- return: ['up', 'down', 'right', 'left', 'in', 'out', 'fixedly', 'unknown']
- [example gestures](https://github.com/NTT-TNN/bkm3t/tree/master/service_predict/test_example.txt)
## activity
- link: localhost:5000/predict_activity
- method: post
- data: list points of data, each point has yaw, pitch, roll, ax, ay, ax, timestamp
- number of data: sent in stream
- return: ['running', 'walking', 'standing', 'sitting', 'laying', 'upstairs', 'downstairs', 'unknown']
- [example activity](https://github.com/NTT-TNN/bkm3t/tree/master/service_predict/test_example.txt)
