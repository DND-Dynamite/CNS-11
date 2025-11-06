# Users and their roles
users = {
"Alice": "Student",
"Bob": "TA",
"Charlie": "Instructor"
}
# Roles and permissions
roles_permissions = {
"Student": ["View_Assignment"],

"TA": ["View_Assignment", "Grade_Assignment"],
"Instructor": ["View_Assignment", "Grade_Assignment", "Modify_Course"]
}
# RBAC access check function
def check_access(user_name, resource_name):
    role = users.get(user_name)
    if not role:
        return f"User {user_name} not found"
    permissions = roles_permissions.get(role, [])
    if resource_name in permissions:
        return f"��Access GRANTED to {user_name} ({role}) for {resource_name}"
    else:
        return f"�Access DENIED to {user_name} ({role}) for {resource_name}"
# Test all scenarios
resources = ["View_Assignment", "Grade_Assignment", "Modify_Course"]
for user in users:
    for resource in resources:
        print(check_access(user, resource))