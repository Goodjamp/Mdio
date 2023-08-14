rmdir /S /Q "../../MdioDelivery"
xcopy MdioDelivery ..\..\MdioDelivery /s /e
copy ..\..\Release\release\Mdio.exe ..\..\MdioDelivery\Mdio.exe