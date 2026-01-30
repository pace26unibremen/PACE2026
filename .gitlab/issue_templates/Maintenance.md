## Description  
_Describe which part of the code or infrastructure needs to be revised._  
<!-- Beispiel: Refaktorierung von Node Indizes zu Node Pointern. -->

## Goal  
_What goal should the adjustment achieve?_  
<!-- Beispiel: Vereinfachung des Zugriffs auf Nodes und deren Umgebung. -->

## Justification  
_Explain why this task is necessary or useful._  
<!-- Beispiel: Der aktuelle Code verwendet Indizes, somit muss für jeden Aufruf einer Node, oder dessen Kind eine Liste aller Nodes aus dem Forest genommen werden, welche mit dem Indize die Node zurückgibt. -->

## Acceptance Criterion  
_Define when the task is considered complete._  
<!-- Beispielsweise: (bitte entsprechend der Situation anpassen -->
- [ ] Refactoring completed and all tests successful  
- [ ] No loss of functionality and no significant change in performance
- [ ] Documentation adapted (if internal structure has been changed)  

## Technical Details (optional)  
_Describe relevant notes, modules to be tested, or compiler requirements.._  
<!-- Beispiel: Beim speichern von Pointern innerhalb vom Typ vector muss minuziös darauf geachtet werden, dass diese nicht resized werden, da die Pointer sonst nicht mehr auf die richtigen Objekte zeigen könnten. -->

## References (optional)  
_Add links or references to issues, merge requests, or analyses._  
<!-- Beispiel: Siehe Issue [Refactor Node Indices to Pointers](#26) zum Refactoring -->
