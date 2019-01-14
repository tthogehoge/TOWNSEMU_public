					ORG		$1800

					BRA		REAL_ENTRY


D77_FROM_PC_RS232C_DATA_PTR				EQU		$2800
D77_FROM_PC_RS232C_FORMAT_IMAGE_PTR		EQU		$3C00


DRIVE_IN			FCB		1				; $1602




REAL_ENTRY			PSHS	A,B,X,Y,U,CC,DP


					; Clearing FM77AV40's drive mapping
					LDA		#$5F
					STA		$FD1E
					LDA		#$5A
					STA		$FD1E
					LDA		#$55
					STA		$FD1E
					LDA		#$50
					STA		$FD1E


					LDA		DRIVE_IN,PCR
					STA		DRIVE,PCR
					STA		FORMAT_DRIVE,PCR


					LBSR	WAIT_FOR_RETURN_KEY


					LDA		DRIVE_IN,PCR
					LBSR	CHECK_DRIVE_NOT_READY
					BCS		D77_FROM_PC_RS232C_DRIVE_NOT_READY_ERROR


					ORCC	#$50
					LBSR	RS232C_OPEN



					LEAX	EMPTY_CMD,PCR
					STX		RS232C_SEND_BUFFER_BEGIN,PCR
					LDX		#2
					STX		RS232C_SEND_BUFFER_SIZE,PCR
					LBSR	RS232C_SEND_BINARY
					LBSR	RS232C_SEND_BINARY
					LBSR	RS232C_SEND_BINARY



					BSR		D77_FORMAT_DISK


					LBSR	PREFORMAT_RESTORE	; Not quite pre-format, but need to restore again.


					CLRA

TRACK_LOOP			PSHS	A
					LBSR	D77_WRITE_SECTORDATA
					PULS	A
					INCA
					CMPA	#80
					BNE		TRACK_LOOP




;					SEND "REQFMT" to PC
;					RECEIVE FORMAT DATA
;					SEND "REQcchh"  (cc:Track hh:Side)
;					RECEIVE TRACK DATA
;					WRITE TRACK

;					CHECKSUM_ERROR -> Re-request



					LEAX	EXIT_CMD,PCR
					STX		RS232C_SEND_BUFFER_BEGIN,PCR
					LDX		#5
					STX		RS232C_SEND_BUFFER_SIZE,PCR
					LBSR	RS232C_SEND_BINARY



					LBSR	RS232C_CLOSE


D77_FROM_PC_RS232C_RTS
					PULS	A,B,X,Y,U,CC,DP,PC


D77_FROM_PC_RS232C_DRIVE_NOT_READY_ERROR
					LEAX	NOTREADY_ERROR_MSG,PCR
					LBSR	PRINT_CSTRING_SUBCPU
					BRA		D77_FROM_PC_RS232C_RTS


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

					; Input A: Drive
CHECK_DRIVE_NOT_READY
					ANDA	#3
					ORA		#$80
					STA		$FD1D		; Motor on

					LDD		#$4000
CHECK_DRIVE_NOT_READY_WAIT
					SUBD	#1
					BNE		CHECK_DRIVE_NOT_READY_WAIT

					LDA		$FD18
					BMI		DRIVE_NOT_READY_DETECTED

					ANDCC	#$FE
					RTS

DRIVE_NOT_READY_DETECTED
					ORCC	#1
					RTS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


D77_FORMAT_DISK		LEAX	REQFMT_CMD,PCR
					STX		RS232C_SEND_BUFFER_BEGIN,PCR

					LDX		#8
					STX		RS232C_SEND_BUFFER_SIZE,PCR

					LBSR	RS232C_SEND_BINARY
					LBSR	RS232C_RECEIVE_FILE

					LBSR	RS232C_OPEN			; RS232C_RECEIVE_FILE closes RS232C connection.  Re-open it.

					LEAX	CHECKSUM_ERROR_MSG,PCR
					TST		RS232C_RECEIVE_FILE_CHECKSUM_ERROR,PCR
					BNE		D77_FORMAT_DISK_ERROR_TRYAGAIN
					LEAX	XOR_ERROR_MSG,PCR
					TST		RS232C_RECEIVE_FILE_XOR_ERROR,PCR
					BNE		D77_FORMAT_DISK_ERROR_TRYAGAIN

					LEAX	ADDRESS_ERROR_MSG,PCR
					LDD		RS232C_RECEIVE_FILE_BEGIN,PCR
					CMPD	#D77_FROM_PC_RS232C_DATA_PTR
					BNE		D77_FORMAT_DISK_ERROR_TRYAGAIN

					STD		FORMAT_TRACKINFO,PCR
					LDX		#D77_FROM_PC_RS232C_FORMAT_IMAGE_PTR
					STX		TRACKIMAGEPTR,PCR

					LDA		$FD0F	; URARAM Off.  FORMAT_DISK_RUN uses BIOS
					LBSR	FORMAT_DISK_RUN

					RTS



D77_FORMAT_DISK_ERROR_TRYAGAIN
					LBSR	PRINT_CSTRING_SUBCPU
					BRA		D77_FORMAT_DISK



CHECKSUM_ERROR_MSG	FCB		"CHECKSUM_ERROR.  RE-TRYING.",$0D,0
XOR_ERROR_MSG		FCB		"XOR_ERROR.  RE-TRYING.",$0D,0
ADDRESS_ERROR_MSG	FCB		"ADDRESS_ERROR.  RE-TRYING.",$0D,0
NOTREADY_ERROR_MSG	FCB		"DRIVE NOT READY.",$0D,0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


					; #Input: X Address to the C-String
PRINT_CSTRING_SUBCPU	PSHS	A,B,Y

					LDD		#0
					TFR		X,Y

PRINT_CSTRING_SUBCPU_COUNT
					INCB
					TST		,Y+
					BNE		PRINT_CSTRING_SUBCPU_COUNT

					DECB
					LBSR	PRINT_SUBCPU

					PULS	A,B,Y,PC


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


D77_WRITE_SECTORDATA
					TFR		A,B

					ANDB	#1
					ADDB	#'0'
					STB		REQTRK_CMD+9,PCR

					LSRA
					LEAX	REQTRK_CMD+6,PCR
					LBSR	ItoA

D77_WRITE_SECTORDATA_RETRY_LOOP
					LBSR	RS232C_OPEN			; RS232C_RECEIVE_FILE closes RS232C connection.  Re-open it.

					LEAX	REQTRK_CMD,PCR
					STX		RS232C_SEND_BUFFER_BEGIN,PCR

					LDX		#12
					STX		RS232C_SEND_BUFFER_SIZE,PCR

					LBSR	RS232C_SEND_BINARY


					LBSR	RS232C_RECEIVE_FILE
					LBSR	RS232C_OPEN			; RS232C_RECEIVE_FILE closes RS232C connection.  Re-open it.

					LEAX	CHECKSUM_ERROR_MSG,PCR
					TST		RS232C_RECEIVE_FILE_CHECKSUM_ERROR,PCR
					BNE		D77_WRITE_SECTORDATA_ERROR_TRYAGAIN
					LEAX	XOR_ERROR_MSG,PCR
					TST		RS232C_RECEIVE_FILE_XOR_ERROR,PCR
					BNE		D77_WRITE_SECTORDATA_ERROR_TRYAGAIN

					LEAX	ADDRESS_ERROR_MSG,PCR
					LDD		RS232C_RECEIVE_FILE_BEGIN,PCR
					CMPD	#D77_FROM_PC_RS232C_DATA_PTR
					BNE		D77_WRITE_SECTORDATA_ERROR_TRYAGAIN

					TFR		D,Y
					LBSR	WRITEDISK_CHUNK

					RTS



D77_WRITE_SECTORDATA_ERROR_TRYAGAIN
					LBSR	PRINT_CSTRING_SUBCPU
					BRA		D77_WRITE_SECTORDATA_RETRY_LOOP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


EMPTY_CMD			FCB		$0D,$0A
REQFMT_CMD			FCB		"REQFMT",$0D,$0A
REQTRK_CMD			FCB		"REQTRK0000",$0D,$0A
EXIT_CMD			FCB		"QQQ",$0D,$0A

