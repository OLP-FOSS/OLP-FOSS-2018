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
- data: list points of data, each point has yaw, pitch, roll, ax, ay, ax, timestamp with format json
- number of data: about 30 point
- return: ['up', 'down', 'right', 'left', 'in', 'out', 'fixedly', 'unknown']
## activity
- link: localhost:5000/predict_activity
- method: post
- data: list points of data, each point has yaw, pitch, roll, ax, ay, ax, timestamp with format json
- number of data: sent in stream
- return: 
    - 1: 'WALKING',
    - 2: 'WALKING_UPSTAIRS',
    - 3: 'WALKING_DOWNSTAIRS',
    - 4: 'SITTING',
    - 5: 'STANDING',
    - 6: 'LAYING',
    - 7: 'STAND_TO_SIT',
    - 8: 'SIT_TO_STAND',
    - 9: 'SIT_TO_LIE',
    - 10: 'LIE_TO_SIT',
    - 11: 'STAND_TO_LIE',
    - 12: 'LIE_TO_STAND'
