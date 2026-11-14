#!/usr/bin/env python3
"""Login qua LDAP (search-then-bind), lấy group, map group → role → permission (RBAC)."""
import sys

from ldap3 import SUBTREE, Connection, Server
from ldap3.core.exceptions import LDAPException
from ldap3.utils.conv import escape_filter_chars

LDAP_URL = "ldap://127.0.0.1"          # production: ldaps:// hoặc StartTLS, verify cert của server
BASE = "dc=netlab,dc=local"
SVC_DN, SVC_PW = "cn=admin,dc=netlab,dc=local", "admin"   # thực tế: service account chỉ có quyền đọc
ROLE_OF_GROUP = {"netops": "operator", "auditors": "viewer"}
PERMS = {"operator": {"config:read", "config:write"}, "viewer": {"config:read"}}
FAIL = "sai username hoặc password"     # một thông báo cho mọi trường hợp: chống user enumeration


def login(username: str, password: str) -> tuple[set[str], set[str]]:
    if not password:
        raise PermissionError(FAIL)                          # chặn unauthenticated bind
    server = Server(LDAP_URL, connect_timeout=3)
    with Connection(server, SVC_DN, SVC_PW, auto_bind=True) as svc:
        svc.search(f"ou=people,{BASE}",
                   f"(&(objectClass=inetOrgPerson)(uid={escape_filter_chars(username)}))",
                   SUBTREE, attributes=["uid"])
        if len(svc.entries) != 1:
            raise PermissionError(FAIL)
        user_dn = svc.entries[0].entry_dn
        svc.search(f"ou=groups,{BASE}",
                   f"(&(objectClass=groupOfNames)(member={escape_filter_chars(user_dn)}))",
                   SUBTREE, attributes=["cn"])
        groups = {str(e.cn) for e in svc.entries}
    user = Connection(server, user_dn, password)
    if not user.bind():                                      # LDAP server tự kiểm tra password
        raise PermissionError(FAIL)
    user.unbind()
    roles = {ROLE_OF_GROUP[g] for g in groups if g in ROLE_OF_GROUP}
    return roles, set().union(*(PERMS[r] for r in roles))


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit("Dùng: ldap_login.py user password action")
    username, password, action = sys.argv[1:]
    try:
        roles, perms = login(username, password)
    except PermissionError as e:
        sys.exit(f"✘ {username}: {e}")
    except LDAPException as e:
        sys.exit(f"✘ lỗi LDAP: {e}")
    decision = "ALLOW" if action in perms else "DENY"       # deny by default
    print(f"✔ {username}: roles={sorted(roles)} permissions={sorted(perms)}")
    print(f"  {action}: {decision}")                         # thực tế: ghi audit log ở đây
