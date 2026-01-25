## Beschreibung  
_Beschreibe, welcher Teil des Codes oder der Infrastruktur überarbeitet werden soll._  
<!-- Beispiel: Refaktorierung von Node Indizes zu Node Pointern. -->

## Ziel  
_Welches Ziel soll die Anpassung erreichen?_  
<!-- Beispiel: Vereinfachung des Zugriffs auf Nodes und deren Umgebung. -->

## Begründung  
_Erkläre, warum diese Aufgabe notwendig oder sinnvoll ist._  
<!-- Beispiel: Der aktuelle Code verwendet Indizes, somit muss für jeden Aufruf einer Node, oder dessen Kind eine Liste aller Nodes aus dem Forest genommen werden, welche mit dem Indize die Node zurückgibt. -->

## Akzeptanzkriterien  
_Definiere, wann die Aufgabe als abgeschlossen gilt._  
<!-- Beispielsweise: (bitte entsprechend der Situation anpassen -->
- [ ] Refaktorierung abgeschlossen und alle Tests erfolgreich  
- [ ] Kein Funktionsverlust und keine signifikante Performanceänderung  
- [ ] Dokumentation angepasst (falls interne Struktur geändert wurde)  

## Technische Details (optional)  
_Beschreibe relevante Hinweise, zu prüfende Module oder Compileranforderungen._  
<!-- Beispiel: Beim speichern von Pointern innerhalb vom Typ vector muss minuziös darauf geachtet werden, dass diese nicht resized werden, da die Pointer sonst nicht mehr auf die richtigen Objekte zeigen könnten. -->

## Referenzen (optional)  
_Füge Links oder Verweise auf Issues, Merge Requests oder Analysen hinzu._  
<!-- Beispiel: Siehe Issue [Refactor Node Indices to Pointers](#26) zum Refactoring -->
