import novapi
from mbuild import gamepad
from mbuild import power_expand_board
from mbuild.smartservo import smartservo_class
from mbuild.encoder_motor import encoder_motor_class

# initialize variables
i = 0
j = 0
k = 0
a = 0
# new class
smartservo_1 = smartservo_class("M6", "INDEX1")
encoder_motor_M1 = encoder_motor_class("M1", "INDEX1")
encoder_motor_M2 = encoder_motor_class("M2", "INDEX1")
encoder_motor_M5 = encoder_motor_class("M5", "INDEX1")
encoder_motor_M4 = encoder_motor_class("M4", "INDEX1")
def AUTON():
    global i, j, k, a
    # Avance

    encoder_motor_M4.set_power(20)
    encoder_motor_M2.set_power(-20)
    encoder_motor_M1.set_power(-20)
    encoder_motor_M5.set_power(20)
    # abre garra y luego la cierra
    power_expand_board.set_power("DC3", 70)
    # sube garra
    power_expand_board.set_power("DC1", 100)
    power_expand_board.set_power("DC2", 100)
    time.sleep(1)
    power_expand_board.set_power("DC1", -38)
    power_expand_board.set_power("DC2", -38)
    encoder_motor_M4.move_to(360, 0)
    encoder_motor_M2.move_to(360, 0)
    encoder_motor_M1.move_to(360, 0)
    encoder_motor_M5.move_to(360, 0)
    time.sleep(.8)
    power_expand_board.set_power("DC1", 0)
    power_expand_board.set_power("DC2", 0)
    time.sleep(1)
    power_expand_board.set_power("DC3", -100)
    time.sleep(1)
    power_expand_board.set_power("DC3", 0)

    
i = 0
j = 0
a = 0
while True:
    if (gamepad.is_key_pressed("≡")):
      AUTON()
    time.sleep(0.001)
    power_expand_board.set_power("DC3", 0)
    power_expand_board.stop("DC1")
    power_expand_board.stop("DC5")
    power_expand_board.stop("DC2")
    # abrir garra
    while gamepad.is_key_pressed("N4"):
      power_expand_board.set_power("DC3", 180)

    # cerrar garra
    while gamepad.is_key_pressed("N1"):
      power_expand_board.set_power("DC3", -150)

    # bajar elevador
    while gamepad.is_key_pressed("Down"):
      power_expand_board.set_power("DC1", -30)
      power_expand_board.set_power("DC2", -30)

    # subir elevador
    while gamepad.is_key_pressed("Up"):
      power_expand_board.set_power("DC1", 115)
      power_expand_board.set_power("DC2", 100)  

    if gamepad.is_key_pressed("R_Thumb"):
        #hace que el motor se sostenga
        power_expand_board.set_power("DC2", 43)
        power_expand_board.set_power("DC1", 43)
        lentorro = .28

    # apaga todo

    # prende la recoleccion de discos
    if gamepad.is_key_pressed("R1"):
      power_expand_board.set_power("DC8", 110)
      power_expand_board.set_power("DC7", 90)
      power_expand_board.set_power("DC6", -90)
      power_expand_board.set_power("DC4", 180)

    # apaga recoleccion de discos
    if gamepad.is_key_pressed("L1"):
      power_expand_board.set_power("DC8", 0)
      power_expand_board.set_power("DC7", 0)
      power_expand_board.set_power("DC6", 0)
      power_expand_board.set_power("DC4", 0)
      power_expand_board.set_power("BL1", 0)

    if gamepad.is_key_pressed("L_Thumb"):
      power_expand_board.set_power("BL2", 50)

    if gamepad.is_key_pressed("N2"):
      power_expand_board.set_power("DC4", 200)

    if gamepad.is_key_pressed("N3"):
      power_expand_board.set_power("DC4", 0)
    #apaga el motor que lleva al disco al disparador
    
    if(gamepad.is_key_pressed("L2")):
        power_expand_board.set_power("BL1", 0)
        power_expand_board.set_power("BL2", 0)
    # expulsa discos
    while (gamepad.is_key_pressed("L2") and gamepad.get_joystick("Ry") > 15):
      power_expand_board.set_power("DC8", -100)
      power_expand_board.set_power("DC7", -100)
      power_expand_board.set_power("DC6", 100)

    while (gamepad.is_key_pressed("L2") and gamepad.get_joystick("Ry") < -15):
      power_expand_board.set_power("DC8", -100)
      power_expand_board.set_power("DC7", -100)
      power_expand_board.set_power("DC6", 100)
      power_expand_board.set_power("DC4", -100)


    # prende disparador
    if gamepad.is_key_pressed("R2"):
      power_expand_board.set_power("BL1", 150)



    # mueve al servomotor para apuntar
    if gamepad.is_key_pressed("Right"):
      smartservo_1.move_to(20, 10)

    else:
      if gamepad.is_key_pressed("Left"):
        smartservo_1.move_to(0, -10)
    deadzone = 15

    Lx = gamepad.get_joystick("Lx")
    Ly = gamepad.get_joystick("Ly")
    Rx = gamepad.get_joystick("Rx")

    Rx = -Rx
    if gamepad.is_key_pressed("+"):
      power_expand_board.set_power("DC2", 35)
      power_expand_board.set_power("DC1", 35)
      lentorro = .25
    else:
      lentorro = 1
        
    # Aplicamos zona muerta
    if abs(Lx) < deadzone: Lx = 0
    if abs(Ly) < deadzone: Ly = 0
    if abs(Rx) < deadzone: Rx = 0

    # Calcular potencias 
    M1 = Ly - Lx - Rx 
    M2 = Ly + Lx - Rx 
    M4 = Ly + Lx + Rx 
    M5 = Ly - Lx + Rx 

    # Normalizar valores (para evitar potencias >100)
    maxPower = max(abs(M1), abs(M2), abs(M4), abs(M5), 100)
    M1 = (M1 / maxPower) * 100
    M2 = (M2 / maxPower) * 100
    M4 = (M4 / maxPower) * 100
    M5 = (M5 / maxPower) * 100

    M1 *= lentorro
    M2 *= lentorro
    M4 *= lentorro
    M5 *= lentorro
    # Corregir sentidos de tus motores según me dijiste
    M1 *= -1  # adelante derecha
    M2 *=-1  # atrás derecha
    M4 *= 1   # adelante izquierda
    M5 *= 1   # atrás izquierda


    # Aplicar potencias
    encoder_motor_M4.set_power(M4)
    encoder_motor_M2.set_power(M2)
    encoder_motor_M1.set_power(M1)
    encoder_motor_M5.set_power(M5)

novapi.reset_timer()
