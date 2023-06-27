# Datenüberabe im Format (Integer)(Char): *xxxxxA* 
 
# Identifiers:
- S: Statusanzeige (rot, grün, blau, gelb/orange / weiß) --> DrawState() 
- L: Loading --> DrawLoading() 
- T/A/I: Positionen (Target/Actual/Inching) --> DrawPosition() 
- I: Positionen (Inching) --> DrawInching() 
- D: DemoMode --> DrawDemo()
- C: Configdata --> no program




# Statusanzeige *xS*
DrawState()
-> 1S: OK (grün)
-> 2S: Error (rot)
-> 3S: Signal1 (blau)
-> 4S: Signal2 (gelb)
-> 5S: Signal3 (weiß)


# Positionen *xA/xT/xI*
DrawPosition()
-> xT: TargePosition (blau)
-> xA: ActualPosition (grün)
-> xI: Inching (Tippbetrieb) (gelb) -> kein Target oder Indicator **ODER** Indicator läuft in Tipp-Richtung 
 
# Loading *xL*
-> 0L: Ladeanzeige beenden -> welcher Status? Nötig?
-> xL: Ladezustand in %, 0/1 - 100, integer
 
# DemoMode *xD*
-> 0D: Start DemoMode, shut down by selecting any other Mode

# Configdata *xC*
-> 0C: Anzeige aus (global)
-> 1C: Anzeige ein (global)
 
Namespace 10-99 Toggle Visualisation
-> 10C: Indicator aus
-> 11C: Indicator ein
-> 20C: Positionierung aus
-> 21C: Positionierung ein
 
Namespace 100 Positionierung
-> 100C: Positionierung disabled  (Kein Lauflicht/Soll/Ist --> für wenn mal erst Config übertragen wird und die positionen dynamisch übertragen werden. Fal wird von der SPS übermittelt wenn fertig)
-> 101C: Positionierung enable    (Kein Lauflicht/Soll/Ist --> für wenn mal erst Config übertragen wird und die positionen dynamisch übertragen werden. Fal wird von der SPS übermittelt wenn fertig)
-> 110C: minPos in mm 
-> 111C: maxPos in mm
 
