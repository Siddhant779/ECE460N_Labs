for x in range(128):
    pfn_part = x<<9
    not_protected = 1<<3
    mapped_valid = 1<<2
    modified = 1<<1
    reference = 1
    if(0<=x<=23):
        final = (pfn_part+mapped_valid) & 0x3e04
        final = "x"+'{0:04x}'.format(final)
        print(".fill "+final)

    if(24<=x<=127):
        final = 0x0000+not_protected
        final = "x"+'{0:04x}'.format(final)
        if(x==24):
            pfn_part=25<<9
            final=(pfn_part+mapped_valid+not_protected) & 0x3e0c
            final = "x"+'{0:04x}'.format(final)
        if(x==96):
            pfn_part=28<<9
            final=(pfn_part+mapped_valid+not_protected) & 0x3e0c
            final = "x"+'{0:04x}'.format(final)
        if(x==126):
            pfn_part=29<<9
            final=(pfn_part+mapped_valid+not_protected) & 0x3e0c
            final = "x"+'{0:04x}'.format(final)
        print(".fill "+final)
