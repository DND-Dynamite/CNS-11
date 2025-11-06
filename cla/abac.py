# Users (students)
students = {
"Alice": {"year": 2, "major": "CS", "GPA": 3.5},
"Bob": {"year": 1, "major": "EE", "GPA": 3.0},
"Charlie": {"year": 3, "major": "CS", "GPA": 2.5},
}
# Resources (course materials)
materials = {
"Intro_CS": {"level": 1, "department": "CS", "min_year": 1, "min_GPA": 2.0},
"Advanced_EE": {"level": 3, "department": "EE", "min_year": 2, "min_GPA": 3.0},
"Math_Open": {"level": 1, "department": "Math", "min_year": 1, "min_GPA": 0.0},
}
# ABAC policy check function
def check_access(student_name, material_name):
    student = students.get(student_name)
    material = materials.get(material_name)
    if not student or not material:
        return "Student or Material not found"
# Year and GPA check
    if student["year"] >= material["min_year"] and student["GPA"] >= material["min_GPA"]:
        if material["department"] == student["major"] or material["department"] == "Math":
            return f"Access GRANTED to {student_name} for {material_name}"
    
    return f"Access DENIED to {student_name} for {material_name}"
# Test all scenarios
for s in students:
    for m in materials:
        print(check_access(s, m))