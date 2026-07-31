# PCI & USB drivers (Linux)

> Hai bus **tự liệt kê được** (self-enumerating) — khác I2C/SPI phải khai trong device tree. Trọng tâm phỏng vấn Embedded Linux có PCI/USB (vd JD Datalogic): device model, config space/descriptor, cách driver probe, interrupt (MSI), DMA, và USB gadget (khi thiết bị *là* USB device). Bổ trợ [driver-basics.md](driver-basics.md).
> Ôn dạng phỏng vấn: bank [DRV-019…027](../15_prep/mock-interview/bank/drivers-embedded.md).

---

## Phần 1 — PCI / PCIe

### 1.1. Vì sao PCI khác I2C/SPI: tự liệt kê

PCI/PCIe **tự mô tả** qua **configuration space** (256 byte, PCIe mở rộng 4KB) mỗi function: **Vendor ID / Device ID**, Class code, và **BAR** (Base Address Register). Kernel **quét bus** lúc boot (bus/device/function — BDF), đọc VID/DID, và **gán địa chỉ** cho các BAR → không cần device tree khai (khác I2C/SPI vốn *không discoverable* nên phải khai trong DT). Đây là ý so sánh hay bị hỏi.

- **BAR**: mỗi BAR yêu cầu một vùng địa chỉ (MMIO hoặc I/O port) kích thước bao nhiêu; kernel/firmware map vào không gian địa chỉ → driver `ioremap` BAR để chạm thanh ghi thiết bị.
- **Config space** truy cập qua `pci_read_config_dword()` v.v.

### 1.2. PCI driver — khung

```c
static const struct pci_device_id my_ids[] = {
    { PCI_DEVICE(0x1234, 0x5678) },   // match theo Vendor/Device ID
    { 0, }
};
MODULE_DEVICE_TABLE(pci, my_ids);      // để udev/kernel match

static int my_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    pci_enable_device(pdev);                       // bật thiết bị
    pci_request_regions(pdev, "mydrv");            // xin quyền các BAR
    void __iomem *regs = pci_iomap(pdev, 0, 0);    // map BAR0 -> MMIO
    pci_set_master(pdev);                          // cho phép device làm bus master (DMA)
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
    // xin IRQ (xem 1.3), đăng ký subsystem...
    return 0;
}
static struct pci_driver my_driver = {
    .name = "mydrv", .id_table = my_ids,
    .probe = my_probe, .remove = my_remove,
};
module_pci_driver(my_driver);
```

Kernel match `id_table` với VID/DID quét được → gọi `probe(pdev)`. Đối xứng probe/remove; ưu tiên `pcim_*`/`devm_*` (managed) để tự dọn.

### 1.3. Interrupt: INTx vs MSI/MSI-X

- **INTx** (legacy): 4 đường ngắt **level-triggered, chia sẻ** — handler phải kiểm tra "có phải của mình không"; ít vector, chia sẻ gây latency.
- **MSI / MSI-X** (message-signaled): thiết bị **ghi một message vào bộ nhớ** thay vì kéo đường IRQ → **không chia sẻ**, nhiều vector riêng (MSI-X tới 2048), edge-like, latency thấp hơn. Hiện đại nên dùng MSI-X.

```c
int n = pci_alloc_irq_vectors(pdev, 1, nvec, PCI_IRQ_MSIX | PCI_IRQ_MSI | PCI_IRQ_INTX);
int irq = pci_irq_vector(pdev, 0);
request_irq(irq, my_isr, 0, "mydrv", dev);   // MSI/MSI-X không cần IRQF_SHARED
```

### 1.4. DMA trên PCI

PCI device là **bus master** — tự đọc/ghi RAM. Driver dùng cùng API DMA ([BSP-011](../15_prep/mock-interview/bank/bsp.md)): `dma_alloc_coherent` (descriptor ring) + `dma_map_single/sg` (payload streaming); `dma_set_mask` khai độ rộng địa chỉ device chịu được. PCIe có thể qua IOMMU (bảo vệ + dịch địa chỉ). Vẫn phải lo cache maintenance nếu không coherent.

---

## Phần 2 — USB

### 2.1. Kiến trúc & descriptor

USB **host-centric**, topology hình sao phân tầng: **host controller** (xHCI cho USB3, EHCI/OHCI cũ) điều phối; thiết bị **không tự nói** trừ khi host hỏi. Cấu trúc mô tả (descriptor) phân cấp:

- **Device descriptor** (VID/PID, class) → **Configuration** → **Interface** (một chức năng, driver bind ở mức *interface*) → **Endpoint** (kênh dữ liệu một chiều).
- **Transfer types** theo endpoint: **Control** (điều khiển/setup), **Bulk** (dữ liệu lớn tin cậy — máy in, storage), **Interrupt** (nhỏ, định kỳ, độ trễ giới hạn — chuột/bàn phím), **Isochronous** (đúng nhịp, không đảm bảo — audio/video).

### 2.2. Enumeration

Cắm vào → host phát hiện (điện trở pull-up), **reset**, gán **address**, đọc descriptor, chọn configuration, **match driver** theo VID/PID hoặc **class** (vd HID, Mass Storage → driver class dùng chung). Sau đó driver dùng endpoint để trao đổi.

### 2.3. USB host driver — URB

Bind ở mức **interface**; giao tiếp bất đồng bộ qua **URB** (USB Request Block):

```c
static struct usb_device_id my_tbl[] = { { USB_DEVICE(0x1234, 0x5678) }, {} };
MODULE_DEVICE_TABLE(usb, my_tbl);

static int my_probe(struct usb_interface *intf, const struct usb_device_id *id) {
    struct usb_device *udev = interface_to_usbdev(intf);
    // tìm endpoint, cấp URB, ...
    return 0;
}
static struct usb_driver my_driver = {
    .name="mydrv", .id_table=my_tbl, .probe=my_probe, .disconnect=my_disc,
};
module_usb_driver(my_driver);
```

- **URB** = mô tả một lần truyền tới một endpoint; `usb_submit_urb()` **bất đồng bộ**, xong thì callback chạy — hợp streaming.
- Bản **đồng bộ** tiện: `usb_control_msg()`, `usb_bulk_msg()` (block tới khi xong/timeout).

### 2.4. USB gadget — khi thiết bị *là* USB device

Quan trọng cho embedded: board của bạn đóng vai **USB device** (cắm vào PC). Dùng **USB gadget framework** + **UDC** (USB Device Controller driver): chọn **function** — `g_serial` (CDC-ACM cổng COM ảo), `g_mass_storage` (ổ USB), `g_ether` (mạng qua USB), HID; hoặc ghép nhiều function bằng **configfs** (composite gadget). OTG/dual-role: board vừa làm host vừa làm device tùy cắm.

### 2.5. Debug USB

`lsusb` / `lsusb -t` (cây thiết bị + driver bind), `dmesg` (enumerate/disconnect), **usbmon** + Wireshark (bắt gói USB), `/sys/kernel/debug/usb`. Lỗi hay gặp: sai endpoint, quên `usb_submit_urb` re-submit cho interrupt-in, thiếu quyền (udev rule), UDC/gadget không match.

---

## So sánh nhanh (hay bị hỏi)

| | I2C/SPI | PCI/PCIe | USB |
|---|---|---|---|
| Discoverable? | ❌ → cần **device tree** | ✅ config space | ✅ descriptor |
| Match driver | `compatible` (DT) | Vendor/Device ID | VID/PID hoặc class |
| Địa chỉ thanh ghi | DT `reg` | **BAR** (kernel gán) | không MMIO — qua endpoint |
| Ngắt | GPIO IRQ (DT) | INTx / **MSI-X** | interrupt endpoint (polled by host) |
| DMA | qua controller | device là **bus master** | host controller lo |

---

## Ôn tập (bank)

[DRV-019…022](../15_prep/mock-interview/bank/drivers-embedded.md) (PCI), [DRV-023…027](../15_prep/mock-interview/bank/drivers-embedded.md) (USB). Nền chung driver: [driver-basics.md](driver-basics.md); DMA/cache: [BSP-011](../15_prep/mock-interview/bank/bsp.md); vì sao I2C/SPI cần device tree: [DRV-007](../15_prep/mock-interview/bank/drivers-embedded.md).
