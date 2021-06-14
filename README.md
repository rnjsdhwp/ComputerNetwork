## README.md

## - 한국산업기술대
## - 컴퓨터네트워크 프로젝트
## - 간단한 혼잡제어(congestion control) 구현

## - 구현 기능
### 1. 서버와 클라이언트 프로그램으로 구성하여 혼잡제어 알고리즘에 의해 전송하는 데이터를 제어
### 2. slow start : 서버와 클라이언트의 연결이 시작되면 CongWin 값을 1부터 시작하여 2배씩 증가
![image](https://user-images.githubusercontent.com/76527391/121825725-e9b26f00-ccee-11eb-9462-535e869de21c.png)
### 3. Timeout 발생 시, 현재 CongWing값의 1/2을 ThreshOld로 설정하고 CongWin은 1로 설정
![image](https://user-images.githubusercontent.com/76527391/121825732-f46d0400-ccee-11eb-8771-c709bda0372e.png)
### 4. 3중복ACK 발생 시, 현재의 CongWin값을 1/2로 설정하고 이 값을 ThreshOld로 설정
![image](https://user-images.githubusercontent.com/76527391/121825739-ffc02f80-ccee-11eb-9868-abd5ee7c3f17.png)

