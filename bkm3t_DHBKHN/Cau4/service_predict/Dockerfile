FROM python:3
ADD . /service_predict
WORKDIR /service_predict
RUN pip3 install -r requirements.txt
CMD python3 service.py
