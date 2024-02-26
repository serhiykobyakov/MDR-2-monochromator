unit MDR;
{
Lomo MDR-2 monochromator unit
Version 11.09.2022

(c) Serhiy Kobyakov
}


interface

uses
  Classes, SysUtils, dialogs, StdCtrls, Controls, Forms,
  IniFiles,
  Math,
  addfunc,
  ArduinoDevice;


type

  { NewDevice_device }

  { MDR_device }

  MDR_device = Object (_ArduinoDevice)
    private
      fMaxPos: Cardinal;
      fPos: Cardinal;
//      frDisp: Extended;
      fGrating: byte;
      fA0: array[0..2] of Extended;
      fA1: array[0..2] of Extended;
      function GetGrating: Byte;
      function GetPosNm: Real;
      procedure SetGrating(AValue: Byte);
      procedure SetPos(pos: Cardinal);
      function NmToPos(posNm: Real): Cardinal;
      function PosToNm(pos: Cardinal): Real;
      function InRange(pos: Cardinal): boolean;

    public
      constructor Init(_ComPort: string);
      destructor Done;

// testing - it belongs to private
      function GetPos: Cardinal;
      procedure GoToPos(pos: Cardinal);

      function InRangeNm(posNm: Real): boolean;
      procedure GoToPosNm(posNm: Real);
      procedure InitGrating;
      function GratingStr: string;

      procedure testProc;

      property PositionNm: Real Read GetPosNm;
      property Grating: Byte Read GetGrating Write SetGrating;
  end;


implementation

function MDR_device.GetPos: Cardinal;
begin
  Result := fPos;
end;

function MDR_device.GetPosNm: Real;
begin
  Result := RoundTo(PosToNm(fPos), -2);
end;

function MDR_device.GetGrating: Byte;
begin
  Result := fGrating;
end;

procedure MDR_device.SetGrating(AValue: Byte);
begin
  if (AValue >= 0) and (AValue <= 2) then fGrating := AValue
  else
    showmessage(theDeviceID + ':' + LineEnding +
                'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
                'got value: ' + 'AValue' + LineEnding +
                'Must be: ' + LineEnding +
                '0 for 1200 lines/mm' + LineEnding +
                '1 for 600 lines/mm' + LineEnding +
                '2 for 300 lines/mm');
end;

procedure MDR_device.SetPos(pos: Cardinal);
var
  answer: Cardinal;
begin
  if (pos <= fMaxPos) then
    begin
      fPos := pos;
      answer := StrToInt(Trim(SendAndGetAnswer('s' + IntToStr(pos))));
      if answer <> pos then
        showmessage(theDeviceID + ':' + LineEnding +
                    'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
                    'sent: ' + 's' + InttoStr(pos)+ LineEnding +
                    'got answer: ' + IntToStr(answer));
    end;
end;

constructor MDR_device.Init(_ComPort: string);
var
  MyForm: TForm;
  MyLabel: TLabel;
  AppIni: TIniFile;
  UpperInitStr, iniFile: string;
begin
// -----------------------------------------------------------------------------
// first things first
// the device ID string with which it responds to '?'
  theDeviceID := 'MDR-2';
// -----------------------------------------------------------------------------

  iniFile := Application.Location + theDeviceID + '.ini';
  If not FileExists(iniFile) then
    begin
      showmessage(theDeviceID + ':' + LineEnding +
          'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
          'File ' + iniFile + 'has not been found!' + LineEnding +
          'Please fix it');
      halt(0);
    end;

// make a splash screen
// which shows initialization process
  MyForm := TForm.Create(nil);
  with MyForm do begin
     Caption := theDeviceID + ' initialization...';
     SetBounds(0, 0, 450, 90); Position:=poDesktopCenter; BorderStyle := bsNone;
     MyForm.Color := $00EEEEEE; end;

  MyLabel := TLabel.Create(MyForm);
  with MyLabel do begin
     Autosize := True; Align := alNone; Alignment := taCenter; Parent := MyForm;
     Visible := True; AnchorVerticalCenterTo(MyForm);
     AnchorHorizontalCenterTo(MyForm); end;

  MyForm.Show; MyForm.BringToFront;
  UpperInitStr := 'Initializing ' + theDeviceID + ':' + LineEnding;

  MyLabel.Caption:= UpperInitStr + 'Reading ' + theDeviceID + '.ini...';
  sleepFor(50); // refresh the Label to see the change

// -----------------------------------------------------------------------------
// Read the device variables from ini file:
  AppIni := TInifile.Create(iniFile);
// device-specific paremeters:
    fMaxPos := AppIni.ReadInteger(theDeviceID, 'MaxPos', 32800);
    fGrating := AppIni.ReadInteger(theDeviceID, 'Grating', 0);
//   frDisp := StrToFloat(AppIni.ReadString(theDeviceID, 'rDisp', '1'));
    fA0[0] := Str2Float(AppIni.ReadString(theDeviceID, 'G1A0', '1'));
    fA1[0] := Str2Float(AppIni.ReadString(theDeviceID, 'G1A1', '0'));
    fA0[1] := Str2Float(AppIni.ReadString(theDeviceID, 'G2A0', '1'));
    fA1[1] := Str2Float(AppIni.ReadString(theDeviceID, 'G2A1', '0'));
    fA0[2] := Str2Float(AppIni.ReadString(theDeviceID, 'G3A0', '1'));
    fA1[2] := Str2Float(AppIni.ReadString(theDeviceID, 'G3A1', '0'));
  AppIni.Free;
// -----------------------------------------------------------------------------

// Use basic device initialization
  MyLabel.Caption:= UpperInitStr + 'Connecting to ' + _ComPort + '...';
  sleepFor(200); // refresh the Label to see the change
  Inherited Init(_ComPort);

// Now, when communication with the device has been established
// we can send him commands
// Here we send those commands which are necessary before
// we start to work with the device

  MyLabel.Caption:= UpperInitStr + 'Done!';
  sleepFor(500); // refresh the Label just to see "Done"
  MyForm.Close;
  FreeAndNil(MyForm);
end;


destructor MDR_device.Done;
var
  MyForm: TForm;
  MyLabel: TLabel;
  AppIni: TIniFile;
  UpperInitStr, iniFile: string;
begin
// make a splash screen
// which shows initialization process
  MyForm := TForm.Create(nil);
  with MyForm do begin
     Caption := theDeviceID + ' initialization...';
     SetBounds(0, 0, 450, 65); Position:=poDesktopCenter; BorderStyle := bsNone;
     MyForm.Color := $003E99FF; end;

  MyLabel := TLabel.Create(MyForm);
  with MyLabel do begin
     Autosize := True; Align := alNone; Alignment := taCenter; Parent := MyForm;
     Visible := True; AnchorVerticalCenterTo(MyForm);
     AnchorHorizontalCenterTo(MyForm); end;

  MyForm.Show; MyForm.BringToFront;

  UpperInitStr := 'Shutdown ' + theDeviceID + ':' + LineEnding;

  MyLabel.Caption:= UpperInitStr + 'Parking grating...';
  sleepFor(50); // refresh the Label to see the change

// Go to "zero" position with grating
  SendCharAndGetAnswer('z');

// Save the grating to ini file
  MyLabel.Caption:= UpperInitStr + 'Saving parameters to ini file...';
  sleepFor(50); // refresh the Label to see the change
  iniFile := Application.Location + theDeviceID + '.ini';
  AppIni := TInifile.Create(iniFile);
    AppIni.WriteInteger(theDeviceID, 'Grating', fGrating);
  AppIni.Free;

  Inherited Done;

  MyLabel.Caption:= UpperInitStr + 'Done!';
  sleepFor(500); // refresh the Label just to see "Done"
  MyForm.Close;
  FreeAndNil(MyForm);
end;


function MDR_device.InRange(pos: Cardinal): boolean;
// check if the 'pos' value in the allowed range
begin
  if (pos >= 0) and (pos <= fMaxPos) then Result := True
  else Result := False;
end;


function MDR_device.InRangeNm(posNm: Real): boolean;
// check if the 'posNm' value in the allowed range
begin
  if (posNm >= fA0[fGrating]) and
     (posNm <= fA0[fGrating] + fMaxPos*fA1[fGrating]) then
       Result := True
  else
    begin
      showmessage(theDeviceID + ':' + LineEnding +
                  'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
                  'value ' + FloatToStr(posNm) + LineEnding +
                  'is out of the range: ' +
                  FloatToStr(RoundTo(fA0[fGrating], -2)) + ' - ' +
                  FloatToStr(RoundTo(fA0[fGrating] + fMaxPos*fA1[fGrating], -2)) +
                  ' nm');
      Result := False;
    end;
end;


procedure MDR_device.GoToPos(pos: Cardinal);
// go to exact position (motor positions)
var
  answer: Cardinal;
begin
  if InRange(pos) then
    begin
      answer := StrToInt(Trim(SendAndGetAnswer('g' + IntToStr(pos))));
      if answer <> pos then
        showmessage(theDeviceID + ':' + LineEnding +
                    'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
                    'sent: ' + 'g' + InttoStr(pos)+ LineEnding +
                    'got answer: ' + IntToStr(answer))
      else fPos := answer;
    end
  else
    showmessage(theDeviceID + ':' + LineEnding +
                'procedure ''' + {$I %CURRENTROUTINE%} + ''' failed!' + LineEnding +
                'value ' + IntToStr(pos) +
                ' is larger than maximum allowed value: ' + IntToStr(fMaxPos));
end;

procedure MDR_device.GoToPosNm(posNm: Real);
// go to exact position (nm)
begin
  if InRangeNm(posNm) then GoToPos(NmToPos(posNm));
end;


function MDR_device.NmToPos(posNm: Real): Cardinal;
// convert nanometers to stepper position
begin
  Result := Round((posNm - fA0[fGrating])/fA1[fGrating]);
end;

function MDR_device.PosToNm(pos: Cardinal): Real;
// convert stepper position to nanometers
begin
  Result := fA0[fGrating] + pos*fA1[fGrating];
end;


procedure MDR_device.InitGrating;
// grating initialization procedure
var
  MyForm: TForm;
  MyLabel: TLabel;
  UpperInitStr: string;
begin
// make a splash screen
// which shows initialization process
  MyForm := TForm.Create(nil);
  with MyForm do begin
     Caption := theDeviceID + ' grating initialization...';
     SetBounds(0, 0, 450, 90); Position:=poDesktopCenter; BorderStyle := bsNone;
     MyForm.Color := $00EEEEEE; end;

  MyLabel := TLabel.Create(MyForm);
  with MyLabel do begin
     Autosize := True; Align := alNone; Alignment := taCenter; Parent := MyForm;
     Visible := True; AnchorVerticalCenterTo(MyForm);
     AnchorHorizontalCenterTo(MyForm); end;

  MyForm.Show; MyForm.BringToFront;

  UpperInitStr := theDeviceID + ':' + LineEnding;

  MyLabel.Caption:= UpperInitStr + 'Grating initialization' + LineEnding +
                    'It may take a couple of minutes' + LineEnding +
                    'Please wait';
  sleepFor(50); // refresh the Label to see the change

  SendCharAndGetAnswer('i');

  MyLabel.Caption:= UpperInitStr + 'Done!';
  sleepFor(500); // refresh the Label just to see "Done"
  MyForm.Close;
  FreeAndNil(MyForm);
end;


procedure MDR_device.testProc;
// for testing purpouses
// will dissapear in the future
begin
  showmessage('Pos:    0 - ' + IntToStr(fMaxPos) + LineEnding +
              FloatToStr(RoundTo(fA0[0], -2)) + ' - ' +
              FloatToStr(RoundTo(fA0[0] + fMaxPos*fA1[0], -2)) + LineEnding +
              FloatToStr(RoundTo(fA0[1], -2)) + ' - ' +
              FloatToStr(RoundTo(fA0[1] + fMaxPos*fA1[1], -2)) + LineEnding +
              FloatToStr(RoundTo(fA0[2], -2)) + ' - ' +
              FloatToStr(RoundTo(fA0[2] + fMaxPos*fA1[2], -2))
              );
end;


function MDR_device.GratingStr: string;
// Get string identifying actual grating
begin
  Case fGrating of
   0: Result := '1200 l/mm';
   1: Result := '600 l/mm';
   2: Result := '300 l/mm';
  else
    Result := 'err';
  end;
end;

end.


