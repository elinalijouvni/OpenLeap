-- Author: Elina Lijouvni
-- License: GPL v3

do

   local DEVICE_ADDRESS = os.getenv("LEAP_DEV_ADDR")

   local function split(str, pat)
      local t = {}
      local fpat = "(.-)" .. pat
      local last_end = 1
      local s, e, cap = str:find(fpat, 1)
      while s do
         if s ~= 1 or cap ~= "" then
            table.insert(t,cap)
         end
         last_end = e+1
         s, e, cap = str:find(fpat, last_end)
      end
      if last_end <= #str then
         cap = str:sub(last_end)
         table.insert(t, cap)
      end
      return t
   end

   local function to_c_byte_array(bytes)
      return "(unsigned char[]){ 0x" .. table.concat( split( tostring( bytes ), ":") , ", 0x") .. " }"
   end

   local function init_listener()

      local ctrl_out_tap = Listener.new("usb", "usb.device_address == " .. DEVICE_ADDRESS .. " and usb.transfer_type == 0x02 and usb.endpoint_number.direction == 0 and usb.setup.bRequest == 1")

      local ctrl_in_tap = Listener.new("usb", "usb.device_address == " .. DEVICE_ADDRESS .. " and usb.transfer_type == 0x02 and usb.endpoint_number.direction == 1 and usb.setup.bRequest == 129")

      local usb_bmRequestType  = Field.new("usb.bmRequestType")
      local usb_setup_bRequest = Field.new("usb.setup.bRequest")
      local usb_setup_wValue   = Field.new("usb.setup.wValue")
      local usb_setup_wIndex   = Field.new("usb.setup.wIndex")
      local usb_capdata        = Field.new("usb.capdata")

      local usb_setup_wLength  = Field.new("usb.setup.wLength")

      function ctrl_out_tap.packet(pinfo,tvb)

         local code_string = "  ret = libusb_control_transfer(ctx->dev_handle, "
         code_string = code_string .. tostring( usb_bmRequestType() ) .. ", "
         code_string = code_string .. tostring( usb_setup_bRequest() ) .. ", "
         code_string = code_string .. tostring( usb_setup_wValue() ) .. ", "
         code_string = code_string .. tostring( usb_setup_wIndex() ) .. ", "
         code_string = code_string .. to_c_byte_array( usb_capdata() ) .. ", "
         code_string = code_string .. #usb_capdata() .. ", "
         code_string = code_string .. "1000);"
         code_string = code_string .. "\n  if (ret != " .. #usb_capdata() .. ") {\n    printf(\"strl out: ret == %i\\n\", ret);\n  }\n"

         print(code_string)

      end

      function ctrl_in_tap.packet(pinfo,tvb)

         local code_string = "  ret = libusb_control_transfer(ctx->dev_handle, "
         code_string = code_string .. tostring( usb_bmRequestType() ) .. ", "
         code_string = code_string .. tostring( usb_setup_bRequest() ) .. ", "
         code_string = code_string .. tostring( usb_setup_wValue() ) .. ", "
         code_string = code_string .. tostring( usb_setup_wIndex() ) .. ", "
         code_string = code_string .. "data, "
         code_string = code_string .. tostring( usb_setup_wLength() ) .. ", "
         code_string = code_string .. "1000);"
         code_string = code_string .. "\n  fprintf_data(stdout, \"ctrl in:\", data, " .. tostring( usb_setup_wLength() ) .. ");\n"
         code_string = code_string .. "\n  if (ret != " .. tostring( usb_setup_wLength() ) .. ") {\n    printf(\"ctrl in: ret == %i\\n\", ret);\n  }\n"

         print(code_string)

      end

   end

   init_listener()

end
