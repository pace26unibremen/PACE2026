## Description  
_Describe which part of the code or infrastructure needs to be revised._  
<!-- Example: Refactoring node indices to node pointers. -->

## Goal  
_What goal should the adjustment achieve?_  
<!-- Example: Simplifying access to nodes and their surroundings. -->

## Justification  
_Explain why this task is necessary or useful._  
<!-- Example: The current code uses indices, so for every access to a node or its child, a list of all nodes in the forest must be used, which returns the node by index. -->

## Acceptance Criterion  
_Define when the task is considered complete._  
<!-- For example: (please adapt to the situation as needed) -->
- [ ] Refactoring completed and all tests successful  
- [ ] No loss of functionality and no significant change in performance  
- [ ] Documentation updated (if the internal structure has been changed)  

## Technical Details (optional)  
_Describe relevant notes, modules to be tested, or compiler requirements._  
<!-- Example: When storing pointers inside a vector type, great care must be taken that it is not resized, because otherwise the pointers might no longer point to the correct objects. -->

## References (optional)  
_Add links or references to issues, merge requests, or analyses._  
<!-- Example: See issue [Refactor Node Indices to Pointers](#26) for the refactoring -->
