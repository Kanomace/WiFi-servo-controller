
int H6 = 25; // 向上
int H7 = 26; // 向下
int H8 = 27; // 停止

void setup() {
  // put your setup code here, to run once:
  pinMode(H6, OUTPUT);
  pinMode(H7, OUTPUT);
  pinMode(H8, OUTPUT);
  digitalWrite(H6, LOW); // 初始关闭继电器
  digitalWrite(H7, LOW); // 初始关闭继电器
  digitalWrite(H8, LOW); // 初始关闭继电器
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(H6, HIGH); // 打开继电器
  digitalWrite(H7, HIGH); // 打开继电器
  digitalWrite(H8, HIGH); // 打开继电器
}
