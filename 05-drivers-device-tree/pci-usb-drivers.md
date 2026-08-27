# PCI & USB drivers (Linux)

> Hai bus **tự liệt kê được** (self-enumerating) — khác I2C/SPI phải khai trong device tree. Trọng tâm phỏng vấn Embedded Linux có PCI/USB (vd JD Datalogic): device model, config space/descriptor, cách driver probe, interrupt (MSI), DMA, và USB gadget (khi thiết bị *là* USB device). Bổ trợ [driver-basics.md](driver-basics.md).
> Ôn dạng phỏng vấn: bank [DRV-019…027](../14-prep/mock-interview/bank/drivers-embedded.md).

---

## Phần 1 — PCI / PCIe

### 1.1. Vì sao PCI khác I2C/SPI: tự liệt kê

PCI/PCIe **tự mô tả** qua **configuration space** (256 byte, PCIe mở rộng 4 KB) mỗi function: **Vendor ID / Device ID**, Class code, và **BAR** (Base Address Register). Kernel **quét bus** lúc boot (bus/device/function — BDF), đọc VID/DID, và **gán địa chỉ** cho các BAR → không cần device tree khai.

- **BAR**: mỗi BAR yêu cầu một vùng địa chỉ (MMIO hoặc I/O port) kích thước bao nhiêu; kernel/firmware map vào không gian địa chỉ → driver `ioremap` BAR để chạm thanh ghi thiết bị.
- **Config space** truy cập qua `pci_read_config_dword()` v.v.

**⭐ Vì sao I2C/SPI KHÔNG làm được điều đó — cơ chế, không phải quy ước:**

| | PCI / USB | I2C / SPI |
|---|---|---|
| Có kênh hỏi danh tính không? | ✅ **Có** — config space (PCI) / descriptor (USB) nằm ở **vị trí chuẩn hoá**, mọi thiết bị đều phải trả lời | ❌ **Không** — protocol chỉ có *"ghi byte tới địa chỉ X"*, **không có khái niệm "anh là ai"** |
| Dò thử được không? | ✅ Quét BDF là thao tác an toàn | 🔴 **Nguy hiểm** — ghi mù vào một địa chỉ I2C lạ có thể **kích hoạt lệnh thật** của chip đó; SPI thì **không có địa chỉ**, chỉ có chân CS |
| ⇒ Hệ quả | Kernel tự dựng được cây thiết bị | **Ai đó phải khai** — đó chính là device tree |

⇒ Device tree tồn tại **không phải vì ARM thích XML**, mà vì **I2C/SPI/memory-mapped không có cơ chế tự khai báo ở tầng protocol**. Ai đó phải nói cho kernel biết *"ở địa chỉ 0x48 trên bus i2c-1 có một cảm biến TMP102"*.

### ⚠️ `self-enumeration` **KHÔNG** phải `hotplug` — chỗ nhầm phổ biến nhất

Hai tính chất **độc lập nhau**, và tính chất **quyết định** cho câu hỏi *"vì sao không cần device tree"* là **cái thứ nhất**:

| | **Self-enumeration** | **Hotplug** |
|---|---|---|
| Nghĩa là | *"Kernel **hỏi được** thiết bị: anh là ai?"* | *"Cắm/rút **lúc đang chạy** vẫn hoạt động"* |
| Quyết định điều gì | **Có cần khai trong DT không** | Có cần xử lý sự kiện add/remove runtime không |
| PCIe trên nhúng | ✅ **có** | ❌ **thường KHÔNG** — chip hàn chết trên board, không ai rút ra |
| USB | ✅ có | ✅ có |
| I2C/SPI | ❌ không | ❌ không |

⇒ **PCIe hàn chết trên board vẫn không cần khai vào device tree** — vì nó *self-enumerating*, dù chẳng bao giờ *hotplug*. Trả lời *"vì chúng hotplug"* là **đúng hiện tượng, sai cơ chế**, và interviewer sẽ hỏi ngay *"thế PCIe hàn chết thì sao?"*

📌 **Câu chốt:** *"Không cần device tree vì chúng **tự khai báo được**, không phải vì chúng cắm rút được. Hai chuyện khác nhau."*

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

PCI device là **bus master** — tự đọc/ghi RAM. Driver dùng cùng API DMA ([BSP-011](../14-prep/mock-interview/bank/bsp.md)): `dma_alloc_coherent` (descriptor ring) + `dma_map_single/sg` (payload streaming); `dma_set_mask` khai độ rộng địa chỉ device chịu được. PCIe có thể qua IOMMU (bảo vệ + dịch địa chỉ). Vẫn phải lo cache maintenance nếu không coherent.

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

## Phần 3 — Khi nào dùng gì (quyết định thiết kế)

### 3.1 Board của bạn là **host** hay **device**?

Câu hỏi đầu tiên, và nó quyết định toàn bộ phần mềm bạn phải viết:

| | Board là **HOST** | Board là **DEVICE** (gadget) |
|---|---|---|
| Ai điều phối | Board của bạn | **Máy kia** (PC) |
| Bạn viết gì | USB **host driver** (URB) hoặc dùng driver class sẵn | **Gadget function** + cấu hình UDC |
| Ví dụ | Board đọc máy quét USB cắm vào nó | 🎯 **Máy quét cắm vào PC** · thiết bị phát console qua USB |
| Phần cứng cần | Cổng host + cấp nguồn cho thiết bị | **UDC** (USB Device Controller) trong SoC |

⭐ **Với thiết bị công nghiệp cầm tay (máy quét, máy đọc mã), vai trò thường là DEVICE** — nên **gadget mới là phần đáng đầu tư**, không phải host driver.

### 3.2 Chọn gadget function nào — đừng viết mới nếu có sẵn

| Nhu cầu | Dùng | Trên PC hiện ra là |
|---|---|---|
| Cổng COM ảo để debug/điều khiển | **`g_serial`** (CDC-ACM) | `/dev/ttyACM0` — **không cần driver riêng** |
| Cho PC truy cập file trên thiết bị | `g_mass_storage` | Ổ USB |
| Mạng qua cáp USB | `g_ether` (CDC-ECM/NCM/RNDIS) | Card mạng |
| Nút bấm / máy quét giả bàn phím | HID | Bàn phím — **cắm là chạy, mọi OS** |
| Nhiều chức năng cùng lúc | **composite qua `configfs`** | Nhiều thiết bị |

⚠️ **Chỉ viết gadget function riêng khi không function chuẩn nào vừa.** Dùng class chuẩn nghĩa là **PC không cần cài driver** — đó thường là yêu cầu sản phẩm quan trọng hơn mọi tối ưu kỹ thuật.

### 3.3 INTx hay MSI/MSI-X?

| | INTx (legacy) | MSI-X |
|---|---|---|
| Cơ chế | Kéo một trong 4 đường IRQ vật lý | Thiết bị **ghi một message vào bộ nhớ** |
| Chia sẻ | ✅ **có** ⇒ handler phải kiểm *"có phải của mình không"*, trả `IRQ_NONE` nếu không | ❌ không — mỗi vector một chủ |
| Số vector | 4, dùng chung cả hệ | tới **2048** |
| Đua với DMA | 🔴 **Có** — ngắt có thể tới **trước khi** dữ liệu DMA vào RAM xong | ✅ Không — message đi **cùng đường** với dữ liệu, tới sau ⇒ đảm bảo thứ tự |

⭐ **Dòng cuối là lý do kỹ thuật thật để chọn MSI-X**, không phải "vì nó mới hơn": nó **loại bỏ một lớp bug đua** giữa ngắt và DMA. Với INTx bạn phải tự đọc thanh ghi để chắc dữ liệu đã tới.

⇒ **Mặc định dùng `PCI_IRQ_MSIX | PCI_IRQ_MSI | PCI_IRQ_INTX`** — xin cái tốt nhất, tự lùi về INTx nếu phần cứng/firmware không hỗ trợ.

---

## Phần 4 — ⚠️ Bẫy thực chiến

**① Nhầm `self-enumeration` với `hotplug`.** Xem §1.1 — đây là lỗi phổ biến nhất và interviewer hay khoan đúng đó (*"thế PCIe hàn chết trên board thì sao?"*).

**② Handler INTx quên trả `IRQ_NONE`.** IRQ chia sẻ mà handler nào cũng trả `IRQ_HANDLED` ⇒ kernel tưởng đã xử lý; nếu thiết bị thật vẫn giữ đường ngắt thì **bão ngắt**, tới ngưỡng kernel in `nobody cared` rồi **tắt luôn IRQ đó** — mọi thiết bị dùng chung đường ngắt chết theo.

**③ Quên `pci_set_master()`** ⇒ DMA im lặng không chạy. Thiết bị không báo lỗi, chỉ là **không có gì xảy ra** — rất tốn thời gian dò.

**④ `dma_set_mask` sai độ rộng.** Khai 64-bit trong khi thiết bị chỉ địa chỉ hoá được 32-bit ⇒ DMA ghi vào **vùng nhớ sai**, hỏng dữ liệu ngẫu nhiên ở chỗ khác. Khai thiếu (32 khi thiết bị chịu được 64) thì chỉ chậm hơn vì phải đi qua bounce buffer.

**⑤ USB interrupt-in quên `usb_submit_urb()` lại trong callback.** Nhận đúng **một** gói rồi im. Triệu chứng: *"chuột chỉ chạy một lần"*.

**⑥ Bind USB driver ở mức thiết bị thay vì mức interface.** USB bind ở **interface** — một thiết bị composite (vd webcam = video + audio) có nhiều interface do **nhiều driver khác nhau** quản.

**⑦ Tưởng cứ cắm USB là được cấp đủ dòng.** Thiết bị phải **khai `bMaxPower`** trong descriptor và host có quyền từ chối. Bug này chỉ lộ ra khi cắm vào hub không nguồn.

**⑧ Rút thiết bị USB giữa lúc URB đang bay.** `disconnect()` phải `usb_kill_urb()` cho **mọi** URB đang chờ, nếu không thì callback chạy trên bộ nhớ đã giải phóng.

**⑨ Không xử lý được `probe` chạy nhiều lần.** PCI hotplug hoặc USB cắm lại ⇒ `probe` gọi lại. Dùng `devm_*`/`pcim_*` để tài nguyên tự dọn theo device, tránh rò qua mỗi lần cắm rút.

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

[DRV-019…022](../14-prep/mock-interview/bank/drivers-embedded.md) (PCI), [DRV-023…027](../14-prep/mock-interview/bank/drivers-embedded.md) (USB). Nền chung driver: [driver-basics.md](driver-basics.md); DMA/cache: [BSP-011](../14-prep/mock-interview/bank/bsp.md); vì sao I2C/SPI cần device tree: [DRV-007](../14-prep/mock-interview/bank/drivers-embedded.md).
